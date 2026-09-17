#include "adapter_palette.h"
#include "../../firmware/arduino/GameBoyRP2C02CRT/sgb_lite_palette.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void put_le16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

static int feed_packet(gbcrt_sgb_lite_decoder_t *decoder, const uint8_t packet[16])
{
    int updates = 0;
    updates += gbcrt_sgb_lite_feed_signal(decoder, GBCRT_SGB_SIGNAL_ARM);
    updates += gbcrt_sgb_lite_feed_signal(decoder, GBCRT_SGB_SIGNAL_START);

    for (unsigned byte = 0; byte < 16; ++byte) {
        for (unsigned bit = 0; bit < 8; ++bit) {
            const unsigned one = (packet[byte] >> bit) & 1u;
            updates += gbcrt_sgb_lite_feed_signal(decoder, GBCRT_SGB_SIGNAL_ARM);
            updates += gbcrt_sgb_lite_feed_signal(
                decoder, one ? GBCRT_SGB_SIGNAL_ONE : GBCRT_SGB_SIGNAL_ZERO);
        }
    }

    updates += gbcrt_sgb_lite_feed_signal(decoder, GBCRT_SGB_SIGNAL_ARM);
    updates += gbcrt_sgb_lite_feed_signal(decoder, GBCRT_SGB_SIGNAL_ZERO);
    return updates;
}

int main(void)
{
    gbcrt_sgb_lite_decoder_t decoder;
    gbcrt_sgb_lite_reset(&decoder);

    const uint16_t colors[4] = {0x7fffu, 0x001fu, 0x03e0u, 0x0000u};

    for (uint8_t command_id = GBCRT_SGB_CMD_PAL01;
         command_id <= GBCRT_SGB_CMD_PAL12;
         ++command_id) {
        uint8_t packet[16] = {0};
        packet[0] = (uint8_t)((command_id << 3) | 1u);
        put_le16(&packet[1], colors[0]);
        put_le16(&packet[3], colors[1]);
        put_le16(&packet[5], colors[2]);
        put_le16(&packet[7], colors[3]);

        assert(feed_packet(&decoder, packet) == 1);
        assert(decoder.palette_valid);
        assert(decoder.last_command_id == command_id);
        assert(decoder.palette_sequence == (uint32_t)command_id + 1u);
        for (unsigned shade = 0; shade < 4u; ++shade) {
            assert(decoder.palette_rgb555[shade] == colors[shade]);
        }

        uint8_t firmware_codes[4];
        gbcrt_sgb_translate_palette(decoder.palette_rgb555, firmware_codes);
        for (unsigned shade = 0; shade < 4u; ++shade) {
            const uint8_t emulator_code =
                adapter_palette_quantize_rgb555(decoder.palette_rgb555[shade]);
            assert(firmware_codes[shade] == emulator_code);
            assert(firmware_codes[shade] < 64u);
            assert(firmware_codes[shade] != 0x0du);
        }
    }

    /* Hardware V1 intentionally rejects multi-packet/non-direct traffic. */
    uint8_t multipacket[16] = {0};
    multipacket[0] = (GBCRT_SGB_CMD_PAL01 << 3) | 2u;
    const uint32_t sequence_before = decoder.palette_sequence;
    assert(feed_packet(&decoder, multipacket) == 0);
    assert(decoder.palette_sequence == sequence_before);

    uint8_t attr_blk[16] = {0};
    attr_blk[0] = (0x04u << 3) | 1u;
    assert(feed_packet(&decoder, attr_blk) == 0);
    assert(decoder.palette_sequence == sequence_before);

    puts("firmware SGB-lite OK: PAL01/PAL23/PAL03/PAL12 and RP2C02 quantizer match emulator");
    return 0;
}
