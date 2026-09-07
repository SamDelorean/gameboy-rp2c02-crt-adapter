#include <Arduino.h>

/*
 * Game Boy DMG / SGB -> RP2350/Pico 2 -> RP2C02 CRT Adapter
 * Firmware V0.1
 *
 * Development environment: Arduino IDE + Arduino-Pico core
 * Target board: Raspberry Pi Pico 2 (RP2350, ARM mode)
 *
 * V0.1 intentionally implements the parts already fixed by the hardware/design
 * documents and leaves the timing-critical PIO/DMA capture/output engines as
 * explicit stubs until bench measurements freeze the exact source sampling edge
 * and RP2C02 EXT timing.
 */

namespace hw {
constexpr uint8_t LD0 = 0;
constexpr uint8_t LD1 = 1;
constexpr uint8_t CP  = 2;
constexpr uint8_t CPL = 3;
constexpr uint8_t ST  = 4;
constexpr uint8_t S   = 5;

constexpr uint8_t EXT0_D0 = 6;
constexpr uint8_t EXT1_D1 = 7;
constexpr uint8_t EXT2_D2 = 8;
constexpr uint8_t EXT3_D3 = 9;
constexpr uint8_t PPU_D4  = 10;
constexpr uint8_t PPU_D5  = 11;
constexpr uint8_t PPU_D6  = 12;
constexpr uint8_t PPU_D7  = 13;
constexpr uint8_t PPU_A12_PAIR = 14;
constexpr uint8_t PPU_A0       = 15;
constexpr uint8_t PPU_nCS      = 16;
constexpr uint8_t PPU_nINT     = 17;

constexpr uint8_t P14 = 18;
constexpr uint8_t P15 = 19;

// GP20/GP21 are reserved for the separate clock-generator sheet (I2C if used).
constexpr uint8_t PALETTE_BUTTON = 22;
}

namespace video {
constexpr uint16_t SOURCE_W = 160;
constexpr uint16_t SOURCE_H = 144;
constexpr uint16_t OUTPUT_W = 256;
constexpr uint16_t OUTPUT_H = 240;
constexpr uint16_t IMAGE_W  = 234;
constexpr uint16_t BORDER_W = 11;
constexpr uint8_t  SHADE_BITS = 2;
constexpr size_t FRAME_BYTES = (SOURCE_W * SOURCE_H * SHADE_BITS) / 8; // 5760
constexpr uint8_t BORDER_EXT_INDEX = 4;
}

namespace ppu {
constexpr uint8_t REG_CTRL = 0; // $2000
constexpr uint8_t REG_MASK = 1; // $2001
constexpr uint8_t REG_ADDR = 6; // $2006
constexpr uint8_t REG_DATA = 7; // $2007

constexpr uint8_t CTRL_NMI_ENABLE = 0x80;
constexpr uint8_t CTRL_EXT_INPUT  = 0x00; // bit 6 clear: EXT pins are inputs
constexpr uint8_t MASK_RENDER_OFF = 0x00;
constexpr uint32_t WARMUP_MS = 35;
}

alignas(4) static uint8_t frameA[video::FRAME_BYTES];
alignas(4) static uint8_t frameB[video::FRAME_BYTES];
static uint8_t *frontFrame = frameA;
static uint8_t *backFrame  = frameB;
static volatile bool backFrameReady = false;

static uint8_t hRepeat[video::SOURCE_W];
static uint8_t vRepeat[video::SOURCE_H];

static volatile uint32_t ppuVblankCounter = 0;
static uint32_t servicedVblankCounter = 0;
static uint8_t extHoldIndex = 0;

struct Palette4 {
  uint8_t color[4];
};

// Provisional palette values. The project will later replace/extend these with
// measured/curated presets. $0F is used as safe black; $0D is intentionally avoided.
static const Palette4 kManualPalettes[] = {
  {{0x30, 0x20, 0x10, 0x0F}}, // neutral grayscale
  {{0x2A, 0x1A, 0x0A, 0x0F}}, // green family (provisional)
  {{0x27, 0x17, 0x07, 0x0F}}, // amber family (provisional)
  {{0x22, 0x12, 0x02, 0x0F}}, // cool blue family (provisional)
};
constexpr uint8_t MANUAL_PALETTE_COUNT = sizeof(kManualPalettes) / sizeof(kManualPalettes[0]);

// paletteMode = 0: AUTO/SGB. 1..N: manual palette 1..N.
static uint8_t paletteMode = 0;
static bool palettePending = true;
static bool cachedSgbValid = false;
static Palette4 cachedSgbPalette = {{0x30, 0x20, 0x10, 0x0F}};
static const Palette4 kAutoFallback = {{0x30, 0x20, 0x10, 0x0F}};

static bool buttonStablePressed = false;
static bool buttonLastRawPressed = false;
static uint32_t buttonLastChangeMs = 0;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;

void onPpuVblank() {
  ++ppuVblankCounter;
}

static inline uint8_t packedGetPixel(const uint8_t *frame, uint16_t x, uint16_t y) {
  const size_t pixel = static_cast<size_t>(y) * video::SOURCE_W + x;
  const size_t byteIndex = pixel >> 2;            // four pixels per byte
  const uint8_t shift = static_cast<uint8_t>((pixel & 3u) * 2u);
  return static_cast<uint8_t>((frame[byteIndex] >> shift) & 0x03u);
}

static inline void packedSetPixel(uint8_t *frame, uint16_t x, uint16_t y, uint8_t shade) {
  const size_t pixel = static_cast<size_t>(y) * video::SOURCE_W + x;
  const size_t byteIndex = pixel >> 2;
  const uint8_t shift = static_cast<uint8_t>((pixel & 3u) * 2u);
  const uint8_t mask = static_cast<uint8_t>(0x03u << shift);
  frame[byteIndex] = static_cast<uint8_t>((frame[byteIndex] & ~mask) | ((shade & 0x03u) << shift));
}

static void buildScalerTables() {
  memset(hRepeat, 0, sizeof(hRepeat));
  memset(vRepeat, 0, sizeof(vRepeat));

  // Center-sampled reference mapping, implemented with integer arithmetic.
  for (uint16_t outX = 0; outX < video::IMAGE_W; ++outX) {
    const uint32_t numerator = (2u * outX + 1u) * video::SOURCE_W;
    const uint16_t srcX = static_cast<uint16_t>(numerator / (2u * video::IMAGE_W));
    ++hRepeat[srcX];
  }

  for (uint16_t outY = 0; outY < video::OUTPUT_H; ++outY) {
    const uint32_t numerator = (2u * outY + 1u) * video::SOURCE_H;
    const uint16_t srcY = static_cast<uint16_t>(numerator / (2u * video::OUTPUT_H));
    ++vRepeat[srcY];
  }
}

static bool scalerTablesValid() {
  uint16_t hTotal = 0;
  uint16_t vTotal = 0;

  for (uint16_t x = 0; x < video::SOURCE_W; ++x) {
    if (hRepeat[x] < 1 || hRepeat[x] > 2) return false;
    hTotal += hRepeat[x];
  }

  for (uint16_t y = 0; y < video::SOURCE_H; ++y) {
    if (vRepeat[y] < 1 || vRepeat[y] > 2) return false;
    vTotal += vRepeat[y];
  }

  return hTotal == video::IMAGE_W && vTotal == video::OUTPUT_H;
}

static void generateStartupTestFrame(uint8_t *frame) {
  memset(frame, 0, video::FRAME_BYTES);
  for (uint16_t y = 0; y < video::SOURCE_H; ++y) {
    for (uint16_t x = 0; x < video::SOURCE_W; ++x) {
      const uint8_t shade = static_cast<uint8_t>((x / 40u) & 0x03u);
      packedSetPixel(frame, x, y, shade);
    }
  }
}

static inline void setExtIndex(uint8_t index) {
  extHoldIndex = static_cast<uint8_t>(index & 0x0Fu);
  digitalWrite(hw::EXT0_D0, (extHoldIndex >> 0) & 1u);
  digitalWrite(hw::EXT1_D1, (extHoldIndex >> 1) & 1u);
  digitalWrite(hw::EXT2_D2, (extHoldIndex >> 2) & 1u);
  digitalWrite(hw::EXT3_D3, (extHoldIndex >> 3) & 1u);
}

// Hooks used by the low-rate PPU host bus. In V0.1 no PIO output engine owns
// GP6..GP9 yet. A later revision will pause/resume the EXT PIO state machine here.
static inline void videoOutputPauseForPpuBus() {}
static inline void videoOutputResumeAfterPpuBus() { setExtIndex(extHoldIndex); }

static void setPpuDataByte(uint8_t value) {
  digitalWrite(hw::EXT0_D0, (value >> 0) & 1u);
  digitalWrite(hw::EXT1_D1, (value >> 1) & 1u);
  digitalWrite(hw::EXT2_D2, (value >> 2) & 1u);
  digitalWrite(hw::EXT3_D3, (value >> 3) & 1u);
  digitalWrite(hw::PPU_D4,  (value >> 4) & 1u);
  digitalWrite(hw::PPU_D5,  (value >> 5) & 1u);
  digitalWrite(hw::PPU_D6,  (value >> 6) & 1u);
  digitalWrite(hw::PPU_D7,  (value >> 7) & 1u);
}

static void ppuWriteRegister(uint8_t reg, uint8_t value) {
  // Hardware V0.1 ties PPU R/W permanently LOW.
  // A1 and A2 are physically tied and are HIGH only for $2006/$2007.
  const bool pairHigh = (reg == ppu::REG_ADDR || reg == ppu::REG_DATA);
  const bool a0High = (reg & 0x01u) != 0;

  videoOutputPauseForPpuBus();
  noInterrupts();

  digitalWrite(hw::PPU_nCS, HIGH);
  digitalWrite(hw::PPU_A12_PAIR, pairHigh ? HIGH : LOW);
  digitalWrite(hw::PPU_A0, a0High ? HIGH : LOW);
  setPpuDataByte(value);

  delayMicroseconds(1); // conservative low-rate setup time for bench bring-up
  digitalWrite(hw::PPU_nCS, LOW);
  delayMicroseconds(1);
  digitalWrite(hw::PPU_nCS, HIGH);
  delayMicroseconds(1);

  interrupts();
  videoOutputResumeAfterPpuBus();
}

static void ppuSetAddress(uint16_t address) {
  ppuWriteRegister(ppu::REG_ADDR, static_cast<uint8_t>(address >> 8));
  ppuWriteRegister(ppu::REG_ADDR, static_cast<uint8_t>(address & 0xFFu));
}

static const Palette4 &activePalette() {
  if (paletteMode == 0) {
    return cachedSgbValid ? cachedSgbPalette : kAutoFallback;
  }
  return kManualPalettes[paletteMode - 1u];
}

static void ppuLoadActivePalette() {
  const Palette4 &pal = activePalette();

  // EXT input selects the low four palette-address bits while rendering is off.
  // Fill $3F00..$3F03 with Game Boy shades and $3F04..$3F0F with safe black.
  ppuSetAddress(0x3F00);
  for (uint8_t i = 0; i < 16; ++i) {
    const uint8_t value = (i < 4) ? pal.color[i] : 0x0F;
    ppuWriteRegister(ppu::REG_DATA, value);
  }

  // Important: leave v outside palette RAM, otherwise rendering-disabled PPU
  // can display the addressed palette entry instead of EXT input.
  ppuSetAddress(0x0000);
  setExtIndex(extHoldIndex);
}

static void ppuInitialize() {
  // /RESET is passive-high in the V0.1 schematic. Wait beyond the documented
  // power-up write-inhibit interval before relying on control/address writes.
  delay(ppu::WARMUP_MS);

  ppuWriteRegister(ppu::REG_CTRL, ppu::CTRL_EXT_INPUT);  // NMI off, EXT input
  ppuWriteRegister(ppu::REG_MASK, ppu::MASK_RENDER_OFF); // force blanking / EXT picture

  ppuLoadActivePalette();
  ppuSetAddress(0x0000);

  // Enable NMI while keeping bit 6 clear so EXT remains input.
  // If this happens during an already-active VBlank, one immediate interrupt is harmless.
  ppuWriteRegister(ppu::REG_CTRL, ppu::CTRL_NMI_ENABLE | ppu::CTRL_EXT_INPUT);
}

static void configurePins() {
  pinMode(hw::LD0, INPUT);
  pinMode(hw::LD1, INPUT);
  pinMode(hw::CP,  INPUT);
  pinMode(hw::CPL, INPUT);
  pinMode(hw::ST,  INPUT);
  pinMode(hw::S,   INPUT);

  // Optional passive SGB-lite inputs. No internal pulls by design.
  pinMode(hw::P14, INPUT);
  pinMode(hw::P15, INPUT);

  pinMode(hw::EXT0_D0, OUTPUT);
  pinMode(hw::EXT1_D1, OUTPUT);
  pinMode(hw::EXT2_D2, OUTPUT);
  pinMode(hw::EXT3_D3, OUTPUT);
  pinMode(hw::PPU_D4, OUTPUT);
  pinMode(hw::PPU_D5, OUTPUT);
  pinMode(hw::PPU_D6, OUTPUT);
  pinMode(hw::PPU_D7, OUTPUT);
  pinMode(hw::PPU_A12_PAIR, OUTPUT);
  pinMode(hw::PPU_A0, OUTPUT);
  pinMode(hw::PPU_nCS, OUTPUT);

  // External pull-up is present in hardware; keep deselected immediately.
  digitalWrite(hw::PPU_nCS, HIGH);
  digitalWrite(hw::PPU_A12_PAIR, LOW);
  digitalWrite(hw::PPU_A0, LOW);
  setPpuDataByte(0x00);
  setExtIndex(0);

  pinMode(hw::PPU_nINT, INPUT); // external 3.3 V pull-up in schematic
  pinMode(hw::PALETTE_BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(hw::PPU_nINT), onPpuVblank, FALLING);
}

static void handlePaletteButton() {
  const bool rawPressed = digitalRead(hw::PALETTE_BUTTON) == LOW;
  const uint32_t now = millis();

  if (rawPressed != buttonLastRawPressed) {
    buttonLastRawPressed = rawPressed;
    buttonLastChangeMs = now;
  }

  if ((now - buttonLastChangeMs) >= BUTTON_DEBOUNCE_MS && rawPressed != buttonStablePressed) {
    buttonStablePressed = rawPressed;
    if (buttonStablePressed) {
      // AUTO/SGB -> manual 1 -> ... -> manual N -> AUTO/SGB
      paletteMode = static_cast<uint8_t>((paletteMode + 1u) % (MANUAL_PALETTE_COUNT + 1u));
      palettePending = true;
      Serial.print("Palette mode -> ");
      if (paletteMode == 0) {
        Serial.println("AUTO/SGB");
      } else {
        Serial.print("manual ");
        Serial.println(paletteMode);
      }
    }
  }
}

static void serviceVblankBoundary() {
  if (backFrameReady) {
    noInterrupts();
    uint8_t *tmp = frontFrame;
    frontFrame = backFrame;
    backFrame = tmp;
    backFrameReady = false;
    interrupts();
  }

  if (palettePending) {
    ppuLoadActivePalette();
    palettePending = false;
  }
}

// ----------------------- Timing-critical engines (V0.2 work) -----------------------

static void captureEngineInit() {
  // TODO V0.2: PIO + DMA capture from LD0/LD1 qualified by measured CP/timing.
  // Bench work must freeze sample edge, active-pixel window, CPL/ST/S usage and
  // handling of the DMG fine-scroll/suppressed-clock behavior before this is coded.
}

static void captureEngineService() {
  // TODO V0.2: mark backFrameReady only after one complete valid 160x144 frame.
}

static void extOutputEngineInit() {
  // TODO V0.2: PIO + DMA output of:
  //   11 border + 234 scaled image + 11 border, 240 lines.
  // V0.1 holds a static EXT index so the RP2C02/palette/composite chain can be
  // validated independently of the pixel engine.
  setExtIndex(0);
}

static void extOutputEngineService() {
  // TODO V0.2: hardware-driven output; no pixel-rate interrupt bit-banging.
  (void)frontFrame;
  (void)packedGetPixel;
}

static void sgbListenerService() {
  // TODO later: passive P14/P15 decoder for PAL01/PAL23/PAL03/PAL12.
  // When valid, populate cachedSgbPalette and set cachedSgbValid=true.
  // Never alter visible palette while a manual mode is selected.
}

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("Game Boy RP2C02 CRT Adapter firmware V0.1");
  Serial.println("Target: Raspberry Pi Pico 2 / RP2350");

  configurePins();
  buildScalerTables();

  if (!scalerTablesValid()) {
    Serial.println("ERROR: scaler-table self-test failed");
  } else {
    Serial.println("Scaler tables OK: 160x144 -> 234x240 + 11/11 border");
  }

  generateStartupTestFrame(frontFrame);
  memset(backFrame, 0, video::FRAME_BYTES);

  captureEngineInit();
  extOutputEngineInit();
  ppuInitialize();

  servicedVblankCounter = ppuVblankCounter;
  Serial.println("PPU initialization issued; waiting for VBlank events");
}

void loop() {
  handlePaletteButton();
  captureEngineService();
  extOutputEngineService();
  sgbListenerService();

  const uint32_t currentVblank = ppuVblankCounter;
  if (currentVblank != servicedVblankCounter) {
    servicedVblankCounter = currentVblank;
    serviceVblankBoundary();
  }
}
