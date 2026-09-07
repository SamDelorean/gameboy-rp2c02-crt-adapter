#pragma once

#include <hardware/pio.h>
#include <hardware/pio_instructions.h>

/*
 * Game Boy DMG / SGB LCD capture PIO program — firmware V0.2
 *
 * Design basis (to be bench-validated):
 * - GP0/GP1 = LD0/LD1, contiguous two-bit pixel field.
 * - GP2 = CP pixel clock.
 * - GP3 = CPL line latch.
 * - GP4 = ST line-start/HSYNC.
 * - GP5 = S frame-start/VSYNC.
 * - A frame starts on the rising edge of S.
 * - A line starts on the rising edge of ST.
 * - 160 CP rising edges are expected for each active line.
 * - The first value shifted by CP is discarded by the physical LCD path.
 * - The visible line is therefore CP samples 1..159 plus the direct LD0/LD1
 *   value present after the 160th CP pulse and before CPL latches the line.
 *
 * The program intentionally samples LD0/LD1 during CP-low immediately BEFORE
 * each CP rising edge. After the 160th CP falling edge it captures the direct
 * final pixel before waiting for CPL high.
 *
 * It emits 11 RX FIFO words per active line:
 *   words 0..9: 160 pre-rising-edge samples, 16 pixels per 32-bit word
 *   word 10:    final direct pixel in bits 1..0
 *
 * Input shift direction is LEFT. Within each 32-bit word, sample 0 is bits
 * 31..30 and sample 15 is bits 1..0. LD0 is bit 0 and LD1 is bit 1 of each
 * two-bit sample because IN PINS preserves ascending GPIO order.
 *
 * No pioasm-generated header is required: instructions are encoded with the
 * Pico SDK pio_encode_* helpers so the Arduino sketch stays self-contained.
 */

namespace gb_capture_pio {

constexpr uint16_t PIXELS_PER_LINE = 160;
constexpr uint16_t ACTIVE_LINES = 144;
constexpr uint16_t PACKED_WORDS_PER_LINE = 10;
constexpr uint16_t WORDS_PER_LINE = 11;
constexpr uint32_t WORDS_PER_FRAME = static_cast<uint32_t>(ACTIVE_LINES) * WORDS_PER_LINE;

// Program instruction indices. JMP destinations are program-relative and are
// relocated by pio_add_program() when the program is loaded.
enum ProgramPc : uint8_t {
  PC_WAIT_S_LOW = 0,
  PC_WAIT_S_HIGH,
  PC_WAIT_ST_HIGH,
  PC_WAIT_CPL_LOW,
  PC_WAIT_CP_LOW,
  PC_SET_X_GROUPS,
  PC_SET_Y_PIXELS,
  PC_SAMPLE_PIXEL,
  PC_WAIT_CP_HIGH,
  PC_WAIT_CP_LOW_INNER,
  PC_LOOP_32_PIXELS,
  PC_LOOP_5_GROUPS,
  PC_SAMPLE_FINAL_DIRECT,
  PC_WAIT_CPL_HIGH,
  PC_PUSH_FINAL_DIRECT,
  PC_WAIT_ST_LOW,
  PC_NEXT_LINE,
  PROGRAM_LENGTH
};

// Five groups x 32 CP pulses = 160 CP pulses per active line.
static const uint16_t programInstructions[PROGRAM_LENGTH] = {
  pio_encode_wait_gpio(false, 5),             //  0: wait S low
  pio_encode_wait_gpio(true,  5),             //  1: wait S rising / next frame
  pio_encode_wait_gpio(true,  4),             //  2: wait ST high / line start
  pio_encode_wait_gpio(false, 3),             //  3: CPL must be low during transfer
  pio_encode_wait_gpio(false, 2),             //  4: enter a stable CP-low phase
  pio_encode_set(pio_x, 4),                   //  5: 5 groups total (4..0)
  pio_encode_set(pio_y, 31),                  //  6: 32 pixels total (31..0)
  pio_encode_in(pio_pins, 2),                 //  7: sample LD0/LD1 before CP rising
  pio_encode_wait_gpio(true, 2),              //  8: wait CP rising
  pio_encode_wait_gpio(false, 2),             //  9: wait CP falling / next data stable
  pio_encode_jmp_y_dec(PC_SAMPLE_PIXEL),      // 10: 32 samples
  pio_encode_jmp_x_dec(PC_SET_Y_PIXELS),      // 11: repeat group five times
  pio_encode_in(pio_pins, 2),                 // 12: final direct LCD pixel
  pio_encode_wait_gpio(true, 3),              // 13: CPL rising latches complete line
  pio_encode_push(false, true),                // 14: push two-bit final pixel word
  pio_encode_wait_gpio(false, 4),             // 15: wait ST low before next line
  pio_encode_jmp(PC_WAIT_ST_HIGH),            // 16: capture next active line
};

static const struct pio_program program = {
  programInstructions,
  PROGRAM_LENGTH,
  -1
};

struct Instance {
  PIO pio = nullptr;
  uint sm = 0;
  uint offset = 0;
  bool initialized = false;
};

static inline bool init(Instance &instance,
                        PIO pio,
                        uint sm,
                        uint pinLd0,
                        uint pinCp,
                        uint pinCpl,
                        uint pinSt,
                        uint pinS) {
  if (!pio_can_add_program(pio, &program)) {
    return false;
  }

  const int loadedOffset = pio_add_program(pio, &program);
  if (loadedOffset < 0) {
    return false;
  }

  instance.pio = pio;
  instance.sm = sm;
  instance.offset = static_cast<uint>(loadedOffset);

  // The six source pins are contiguous in the project schematic: GP0..GP5.
  for (uint pin = pinLd0; pin <= pinS; ++pin) {
    pio_gpio_init(pio, pin);
  }
  pio_sm_set_consecutive_pindirs(pio, sm, pinLd0, pinS - pinLd0 + 1u, false);

  pio_sm_config c = pio_get_default_sm_config();
  sm_config_set_wrap(&c, instance.offset + PC_WAIT_S_LOW,
                         instance.offset + PC_NEXT_LINE);
  sm_config_set_in_pins(&c, pinLd0);
#if PICO_PIO_VERSION > 0
  sm_config_set_in_pin_count(&c, 2);
#endif
  sm_config_set_in_shift(&c, false, true, 32); // LEFT shift, autopush every 16 pixels
  sm_config_set_jmp_pin(&c, pinCpl);
  sm_config_set_clkdiv(&c, 1.0f);

  // WAIT GPIO instructions above use the project-fixed raw GPIO numbers.
  // These assertions are intentionally simple because V0.2 targets Pico 2
  // with the canonical GP0..GP5 source mapping.
  if (pinLd0 != 0 || pinCp != 2 || pinCpl != 3 || pinSt != 4 || pinS != 5) {
    pio_remove_program(pio, &program, instance.offset);
    instance = Instance{};
    return false;
  }

  if (pio_sm_init(pio, sm, instance.offset + PC_WAIT_S_LOW, &c) < 0) {
    pio_remove_program(pio, &program, instance.offset);
    instance = Instance{};
    return false;
  }

  instance.initialized = true;
  return true;
}

static inline void prepareForNextFrame(const Instance &instance) {
  pio_sm_set_enabled(instance.pio, instance.sm, false);
  pio_sm_clear_fifos(instance.pio, instance.sm);
  pio_sm_restart(instance.pio, instance.sm);
  pio_sm_exec(instance.pio, instance.sm,
              pio_encode_jmp(instance.offset + PC_WAIT_S_LOW));
}

static inline void start(const Instance &instance) {
  pio_sm_set_enabled(instance.pio, instance.sm, true);
}

static inline void stop(const Instance &instance) {
  pio_sm_set_enabled(instance.pio, instance.sm, false);
}

} // namespace gb_capture_pio
