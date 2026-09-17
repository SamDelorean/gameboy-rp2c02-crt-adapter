#include "sgb_lite.h"

#include <string.h>

enum {
    SGB_CMD_PAL01 = 0x00,
    SGB_CMD_PAL23 = 0x01,
    SGB_CMD_PAL03 = 0x02,
    SGB_CMD_PAL12 = 0x03,
};

static void clear_command(sgb_lite_decoder_t *decoder)
{
    decoder->bit_index = 0;
    decoder->ready_for_stop = false;
    memset(decoder->command, 0, sizeof(decoder->command));
}

void sgb_lite_reset(sgb_lite_decoder_t *decoder)
{
    if (!decoder) return;
    memset(decoder, 0, sizeof(*decoder));
}

static uint16_t le16_at(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static bool decode_direct_palette(sgb_lite_decoder_t *decoder)
{
    const uint8_t command_id = decoder->command[0] >> 3;
    if (command_id > SGB_CMD_PAL12) return false;

    /*
     * V1 SGB-lite intentionally ignores regional attribute commands. A direct
     * PALxx command defines two SGB palettes; for a single global four-color
     * output, use the first palette named by that command. Its common color 0
     * is in bytes 1..2 and its remaining three RGB555 colors are in 3..8.
     *
     * PAL01 -> palette 0
     * PAL23 -> palette 2
     * PAL03 -> palette 0
     * PAL12 -> palette 1
     *
     * The byte layout of the first palette is identical for all four commands,
     * so the global extraction itself is the same.
     */
    decoder->palette_rgb555[0] = le16_at(&decoder->command[1]);
    decoder->palette_rgb555[1] = le16_at(&decoder->command[3]);
    decoder->palette_rgb555[2] = le16_at(&decoder->command[5]);
    decoder->palette_rgb555[3] = le16_at(&decoder->command[7]);
    decoder->palette_valid = true;
    decoder->last_command_id = command_id;
    decoder->palette_sequence++;
    return true;
}

static unsigned expected_command_bits(const sgb_lite_decoder_t *decoder)
{
    unsigned packets = decoder->command[0] & 7u;
    if (packets == 0u) packets = 1u;
    return packets * SGB_LITE_PACKET_BYTES * 8u;
}

bool sgb_lite_feed_joyp(sgb_lite_decoder_t *decoder, uint8_t joyp_value)
{
    if (!decoder) return false;

    const unsigned signal = (joyp_value >> 4) & 3u;
    const unsigned packet_bits = SGB_LITE_PACKET_BYTES * 8u;
    bool palette_updated = false;

    switch (signal) {
    case 3u: /* P14/P15 both high: arm the next pulse. */
        decoder->ready_for_pulse = true;
        break;

    case 2u: /* Zero bit, or the zero-valued stop pulse after 128 bits. */
        if (!decoder->ready_for_pulse || !decoder->ready_for_write) break;

        if (decoder->ready_for_stop) {
            if (decoder->bit_index == expected_command_bits(decoder)) {
                palette_updated = decode_direct_palette(decoder);
                clear_command(decoder);
            }
            decoder->ready_for_pulse = false;
            decoder->ready_for_write = false;
            decoder->ready_for_stop = false;
        }
        else if (decoder->bit_index < SGB_LITE_MAX_BYTES * 8u) {
            decoder->bit_index++;
            decoder->ready_for_pulse = false;
            if ((decoder->bit_index & (packet_bits - 1u)) == 0u) {
                decoder->ready_for_stop = true;
            }
        }
        break;

    case 1u: /* One bit. */
        if (!decoder->ready_for_pulse || !decoder->ready_for_write) break;

        if (decoder->ready_for_stop) {
            /* A stop pulse must be zero; a one here corrupts the command. */
            decoder->ready_for_pulse = false;
            decoder->ready_for_write = false;
            clear_command(decoder);
        }
        else if (decoder->bit_index < SGB_LITE_MAX_BYTES * 8u) {
            decoder->command[decoder->bit_index / 8u] |=
                (uint8_t)(1u << (decoder->bit_index & 7u));
            decoder->bit_index++;
            decoder->ready_for_pulse = false;
            if ((decoder->bit_index & (packet_bits - 1u)) == 0u) {
                decoder->ready_for_stop = true;
            }
        }
        break;

    case 0u: /* Start/reset pulse, or start of a following packet. */
        if (!decoder->ready_for_pulse) break;

        decoder->ready_for_write = true;
        decoder->ready_for_pulse = false;

        if ((decoder->bit_index & (packet_bits - 1u)) != 0u ||
            decoder->bit_index == 0u ||
            decoder->ready_for_stop) {
            clear_command(decoder);
        }
        break;
    }

    return palette_updated;
}
