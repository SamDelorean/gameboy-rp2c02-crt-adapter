#ifndef GBCRT_FIRMWARE_SGB_LITE_PALETTE_H
#define GBCRT_FIRMWARE_SGB_LITE_PALETTE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
 * Minimal passive Super Game Boy palette listener for the physical adapter.
 *
 * Scope is intentionally narrow:
 *   - observe P14/P15 only;
 *   - accept one-packet PAL01/PAL23/PAL03/PAL12 commands;
 *   - extract one global four-color RGB555 palette;
 *   - quantize those four colors to RP2C02 $00-$3F codes.
 *
 * No regional attributes, border transfers, multiplayer responses, or active
 * JOYP emulation are implemented here.
 */

enum {
  GBCRT_SGB_SIGNAL_START = 0u, /* P14/P15 = 00 */
  GBCRT_SGB_SIGNAL_ONE   = 1u, /* P14/P15 = 01 */
  GBCRT_SGB_SIGNAL_ZERO  = 2u, /* P14/P15 = 10 */
  GBCRT_SGB_SIGNAL_ARM   = 3u, /* P14/P15 = 11 */
};

enum {
  GBCRT_SGB_CMD_PAL01 = 0x00u,
  GBCRT_SGB_CMD_PAL23 = 0x01u,
  GBCRT_SGB_CMD_PAL03 = 0x02u,
  GBCRT_SGB_CMD_PAL12 = 0x03u,
};

typedef struct {
  uint8_t command[16];
  uint16_t bit_index;
  bool ready_for_pulse;
  bool ready_for_write;
  bool ready_for_stop;
  bool palette_valid;
  uint16_t palette_rgb555[4];
  uint8_t last_command_id;
  uint32_t palette_sequence;
} gbcrt_sgb_lite_decoder_t;

static inline void gbcrt_sgb_lite_clear_packet(gbcrt_sgb_lite_decoder_t *decoder) {
  decoder->bit_index = 0;
  decoder->ready_for_stop = false;
  memset(decoder->command, 0, sizeof(decoder->command));
}

static inline void gbcrt_sgb_lite_reset(gbcrt_sgb_lite_decoder_t *decoder) {
  if (!decoder) return;
  memset(decoder, 0, sizeof(*decoder));
}

static inline uint16_t gbcrt_sgb_lite_le16(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline bool gbcrt_sgb_lite_decode_palette(gbcrt_sgb_lite_decoder_t *decoder) {
  const uint8_t command_id = decoder->command[0] >> 3;
  const uint8_t packet_count = decoder->command[0] & 7u;
  if (packet_count != 1u || command_id > GBCRT_SGB_CMD_PAL12) return false;

  /* For V1 the first palette carried by PALxx becomes the one global palette. */
  decoder->palette_rgb555[0] = gbcrt_sgb_lite_le16(&decoder->command[1]);
  decoder->palette_rgb555[1] = gbcrt_sgb_lite_le16(&decoder->command[3]);
  decoder->palette_rgb555[2] = gbcrt_sgb_lite_le16(&decoder->command[5]);
  decoder->palette_rgb555[3] = gbcrt_sgb_lite_le16(&decoder->command[7]);
  decoder->palette_valid = true;
  decoder->last_command_id = command_id;
  decoder->palette_sequence++;
  return true;
}

/* Feed the two-bit physical line state: bit 0=P14, bit 1=P15. */
static inline bool gbcrt_sgb_lite_feed_signal(gbcrt_sgb_lite_decoder_t *decoder,
                                               uint8_t signal) {
  if (!decoder) return false;
  signal &= 3u;

  switch (signal) {
    case GBCRT_SGB_SIGNAL_ARM:
      decoder->ready_for_pulse = true;
      break;

    case GBCRT_SGB_SIGNAL_ZERO:
      if (!decoder->ready_for_pulse || !decoder->ready_for_write) break;
      if (decoder->ready_for_stop) {
        const bool updated = decoder->bit_index == 128u &&
                             gbcrt_sgb_lite_decode_palette(decoder);
        decoder->ready_for_pulse = false;
        decoder->ready_for_write = false;
        gbcrt_sgb_lite_clear_packet(decoder);
        return updated;
      }
      if (decoder->bit_index < 128u) {
        decoder->bit_index++;
        decoder->ready_for_pulse = false;
        if (decoder->bit_index == 128u) decoder->ready_for_stop = true;
      }
      break;

    case GBCRT_SGB_SIGNAL_ONE:
      if (!decoder->ready_for_pulse || !decoder->ready_for_write) break;
      if (decoder->ready_for_stop) {
        decoder->ready_for_pulse = false;
        decoder->ready_for_write = false;
        gbcrt_sgb_lite_clear_packet(decoder);
        break;
      }
      if (decoder->bit_index < 128u) {
        decoder->command[decoder->bit_index / 8u] |=
            (uint8_t)(1u << (decoder->bit_index & 7u));
        decoder->bit_index++;
        decoder->ready_for_pulse = false;
        if (decoder->bit_index == 128u) decoder->ready_for_stop = true;
      }
      break;

    case GBCRT_SGB_SIGNAL_START:
      if (!decoder->ready_for_pulse) break;
      decoder->ready_for_write = true;
      decoder->ready_for_pulse = false;
      if (decoder->bit_index != 0u || decoder->ready_for_stop) {
        gbcrt_sgb_lite_clear_packet(decoder);
      }
      break;
  }

  return false;
}

typedef struct {
  uint8_t r, g, b;
} gbcrt_sgb_rgb8_t;

static inline gbcrt_sgb_rgb8_t gbcrt_sgb_rp2c02_rgb(uint8_t code) {
  static const gbcrt_sgb_rgb8_t lut[64] = {
    {84,84,84},{0,30,116},{8,16,144},{48,0,136},{68,0,100},{92,0,48},{84,4,0},{60,24,0},
    {32,42,0},{8,58,0},{0,64,0},{0,60,0},{0,50,60},{0,0,0},{0,0,0},{0,0,0},
    {152,150,152},{8,76,196},{48,50,236},{92,30,228},{136,20,176},{160,20,100},{152,34,32},{120,60,0},
    {84,90,0},{40,114,0},{8,124,0},{0,118,40},{0,102,120},{0,0,0},{0,0,0},{0,0,0},
    {236,238,236},{76,154,236},{120,124,236},{176,98,236},{228,84,236},{236,88,180},{236,106,100},{212,136,32},
    {160,170,0},{116,196,0},{76,208,32},{56,204,108},{56,180,204},{60,60,60},{0,0,0},{0,0,0},
    {236,238,236},{168,204,236},{188,188,236},{212,178,236},{236,174,236},{236,174,212},{236,180,176},{228,196,144},
    {204,210,120},{180,222,120},{168,226,144},{152,226,180},{160,214,228},{160,162,160},{0,0,0},{0,0,0},
  };
  return lut[code & 0x3fu];
}

static inline uint8_t gbcrt_sgb_expand5(uint16_t value) {
  value &= 0x1fu;
  return (uint8_t)((value << 3) | (value >> 2));
}

static inline uint8_t gbcrt_sgb_quantize_rgb555(uint16_t rgb555) {
  const int r = gbcrt_sgb_expand5(rgb555);
  const int g = gbcrt_sgb_expand5(rgb555 >> 5);
  const int b = gbcrt_sgb_expand5(rgb555 >> 10);
  uint32_t best_error = 0xffffffffu;
  uint8_t best_code = 0x0fu;

  for (uint8_t code = 0; code < 64u; ++code) {
    if (code == 0x0du) continue; /* Avoid problematic NTSC $0D on hardware. */
    const gbcrt_sgb_rgb8_t candidate = gbcrt_sgb_rp2c02_rgb(code);
    const int dr = r - candidate.r;
    const int dg = g - candidate.g;
    const int db = b - candidate.b;
    const uint32_t error = (uint32_t)(dr * dr + dg * dg + db * db);
    if (error < best_error) {
      best_error = error;
      best_code = code;
    }
  }
  return best_code;
}

static inline void gbcrt_sgb_translate_palette(const uint16_t rgb555[4],
                                                uint8_t rp2c02[4]) {
  for (uint8_t shade = 0; shade < 4u; ++shade) {
    rp2c02[shade] = gbcrt_sgb_quantize_rgb555(rgb555[shade]);
  }
}

#endif
