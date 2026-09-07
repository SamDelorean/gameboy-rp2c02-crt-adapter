#pragma once

#include <Arduino.h>
#include <hardware/dma.h>
#include <hardware/irq.h>

#include "gb_capture_pio.h"

/*
 * DMA-backed Game Boy DMG / SGB capture engine — firmware V0.2
 *
 * This engine captures one raw 144-line frame from the PIO RX FIFO and then
 * normalizes the LCD shift-register representation into the project's packed
 * 160x144x2-bit framebuffer format.
 *
 * The PIO is deliberately stopped after 144 active lines by the DMA-complete
 * interrupt. The main loop normalizes the frame and re-arms capture while the
 * Game Boy is in vertical blank. If the presentation BACK buffer is still
 * pending, the newly captured raw frame is dropped rather than overwriting a
 * frame awaiting the next PPU VBlank swap.
 */

namespace gb_capture {

alignas(4) static uint32_t rawFrame[gb_capture_pio::WORDS_PER_FRAME];

static gb_capture_pio::Instance pioInstance;
static int dmaChannel = -1;
static dma_channel_config dmaConfig;
static volatile bool rawFrameReady = false;
static volatile bool initialized = false;
static volatile uint32_t rawFramesCompleted = 0;
static uint32_t framesAccepted = 0;
static uint32_t framesDropped = 0;

static inline uint32_t encodedFrameTransferCount() {
  return dma_encode_transfer_count(gb_capture_pio::WORDS_PER_FRAME);
}

static inline uint8_t rawPreClockSample(const uint32_t *lineWords, uint16_t sampleIndex) {
  // Shift-left PIO packing: first two-bit sample is bits 31..30, last is 1..0.
  const uint16_t wordIndex = sampleIndex >> 4;       // /16
  const uint8_t slot = static_cast<uint8_t>(sampleIndex & 0x0Fu);
  const uint8_t shift = static_cast<uint8_t>(30u - 2u * slot);
  return static_cast<uint8_t>((lineWords[wordIndex] >> shift) & 0x03u);
}

static inline void normalizeRawFrameToPacked(uint8_t *destination) {
  // Physical LCD behavior documented by existing captures:
  // visible pixels 0..158 = pre-CP samples 1..159
  // visible pixel 159    = direct LD0/LD1 sample after the 160th CP pulse.
  for (uint16_t y = 0; y < gb_capture_pio::ACTIVE_LINES; ++y) {
    const uint32_t *line = &rawFrame[static_cast<uint32_t>(y) * gb_capture_pio::WORDS_PER_LINE];
    uint8_t *dst = destination + static_cast<uint32_t>(y) * 40u;

    for (uint8_t byteIndex = 0; byteIndex < 39; ++byteIndex) {
      const uint16_t visibleX = static_cast<uint16_t>(byteIndex) * 4u;
      const uint8_t p0 = rawPreClockSample(line, visibleX + 1u);
      const uint8_t p1 = rawPreClockSample(line, visibleX + 2u);
      const uint8_t p2 = rawPreClockSample(line, visibleX + 3u);
      const uint8_t p3 = rawPreClockSample(line, visibleX + 4u);
      dst[byteIndex] = static_cast<uint8_t>(p0 | (p1 << 2) | (p2 << 4) | (p3 << 6));
    }

    const uint8_t p156 = rawPreClockSample(line, 157);
    const uint8_t p157 = rawPreClockSample(line, 158);
    const uint8_t p158 = rawPreClockSample(line, 159);
    const uint8_t p159 = static_cast<uint8_t>(line[gb_capture_pio::PACKED_WORDS_PER_LINE] & 0x03u);
    dst[39] = static_cast<uint8_t>(p156 | (p157 << 2) | (p158 << 4) | (p159 << 6));
  }
}

static void dmaIrqHandler() {
  if (dmaChannel < 0 || !dma_channel_get_irq1_status(static_cast<uint>(dmaChannel))) {
    return;
  }

  dma_channel_acknowledge_irq1(static_cast<uint>(dmaChannel));
  gb_capture_pio::stop(pioInstance);
  rawFrameReady = true;
  ++rawFramesCompleted;
}

static inline void armNextFrame() {
  if (!initialized || dmaChannel < 0) {
    return;
  }

  rawFrameReady = false;
  gb_capture_pio::prepareForNextFrame(pioInstance);

  dma_channel_set_write_addr(static_cast<uint>(dmaChannel), rawFrame, false);
  dma_channel_set_transfer_count(static_cast<uint>(dmaChannel),
                                 encodedFrameTransferCount(),
                                 false);
  dma_channel_start(static_cast<uint>(dmaChannel));

  // DMA is ready before the PIO is allowed to see the next frame-start edge.
  gb_capture_pio::start(pioInstance);
}

static inline bool init(uint pinLd0,
                        uint pinCp,
                        uint pinCpl,
                        uint pinSt,
                        uint pinS) {
  // Request gracefully rather than panicking if another library has consumed
  // all PIO state machines or DMA channels.
  const int claimedSm = pio_claim_unused_sm(pio0, false);
  if (claimedSm < 0) {
    return false;
  }

  if (!gb_capture_pio::init(pioInstance, pio0, static_cast<uint>(claimedSm),
                            pinLd0, pinCp, pinCpl, pinSt, pinS)) {
    pio_sm_unclaim(pio0, static_cast<uint>(claimedSm));
    return false;
  }

  dmaChannel = dma_claim_unused_channel(false);
  if (dmaChannel < 0) {
    gb_capture_pio::stop(pioInstance);
    pio_remove_program(pio0, &gb_capture_pio::program, pioInstance.offset);
    pio_sm_unclaim(pio0, static_cast<uint>(claimedSm));
    pioInstance = gb_capture_pio::Instance{};
    return false;
  }

  dmaConfig = dma_channel_get_default_config(static_cast<uint>(dmaChannel));
  channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_32);
  channel_config_set_read_increment(&dmaConfig, false);
  channel_config_set_write_increment(&dmaConfig, true);
  channel_config_set_dreq(&dmaConfig,
                          pio_get_dreq(pioInstance.pio, pioInstance.sm, false));

  dma_channel_configure(static_cast<uint>(dmaChannel),
                        &dmaConfig,
                        rawFrame,
                        &pioInstance.pio->rxf[pioInstance.sm],
                        encodedFrameTransferCount(),
                        false);

  dma_channel_set_irq1_enabled(static_cast<uint>(dmaChannel), true);
  irq_add_shared_handler(DMA_IRQ_1,
                         dmaIrqHandler,
                         PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
  irq_set_enabled(DMA_IRQ_1, true);

  initialized = true;
  armNextFrame();
  return true;
}

// Returns true when one completed raw frame was consumed by software.
static inline bool service(uint8_t *backFrame, volatile bool &backFramePending) {
  if (!rawFrameReady) {
    return false;
  }

  // The DMA IRQ has stopped the PIO, so rawFrame is stable here.
  rawFrameReady = false;

  if (!backFramePending) {
    normalizeRawFrameToPacked(backFrame);
    backFramePending = true;
    ++framesAccepted;
  } else {
    // Never overwrite a BACK buffer that still awaits the PPU VBlank swap.
    ++framesDropped;
  }

  armNextFrame();
  return true;
}

static inline uint32_t completedCount() { return rawFramesCompleted; }
static inline uint32_t acceptedCount() { return framesAccepted; }
static inline uint32_t droppedCount() { return framesDropped; }
static inline bool isInitialized() { return initialized; }

} // namespace gb_capture
