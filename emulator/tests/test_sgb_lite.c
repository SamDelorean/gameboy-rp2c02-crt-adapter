#include "sgb_lite.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int send_packet(sgb_lite_decoder_t *decoder, const uint8_t packet[16])
{
    int updates = 0;

    /* Reset/start pulse: 11 -> 00. */
    updates += sgb_lite_feed_joyp(decoder, 0x30u);
    updates += sgb_lite_feed_joyp(decoder, 0x00u);

    for (unsigned byte = 0; byte < 16; ++byte) {
        for (unsigned bit = 0; bit < 8; ++bit) {
            const unsigned one = (packet[byte] >> bit) & 1u;
            updates += sgb_lite_feed_joyp(decoder, 0x30u);
            updates += sgb_lite_feed_joyp(decoder, one ? 0x10u : 0x20u);
        }
    }

    /* Stop pulse is zero after another 11 arm state. */
    updates += sgb_lite_feed_joyp(decoder, 0x30u);
    updates += sgb_lite_feed_joyp(decoder, 0x20u);
    return updates;
}

static void put_le16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

int main(void)
{
    sgb_lite_decoder_t decoder;
    sgb_lite_reset(&decoder);
    assert(!decoder.palette_valid);
    assert(decoder.palette_sequence == 0u);

    uint8_t pal01[16] = {0};
    pal01[0] = (0x00u << 3) | 1u;
    put_le16(&pal01[1], 0x7fffu);
    put_le16(&pal01[3], 0x4210u);
    put_le16(&pal01[5], 0x2108u);
    put_le16(&pal01[7], 0x0000u);
    /* Second SGB palette is deliberately different and must not become the
       single global V1 SGB-lite palette. */
    put_le16(&pal01[9], 0x001fu);
    put_le16(&pal01[11], 0x03e0u);
    put_le16(&pal01[13], 0x7c00u);

    assert(send_packet(&decoder, pal01) == 1);
    assert(decoder.palette_valid);
    assert(decoder.last_command_id == 0x00u);
    assert(decoder.palette_sequence == 1u);
    assert(decoder.palette_rgb555[0] == 0x7fffu);
    assert(decoder.palette_rgb555[1] == 0x4210u);
    assert(decoder.palette_rgb555[2] == 0x2108u);
    assert(decoder.palette_rgb555[3] == 0x0000u);

    uint8_t pal23[16] = {0};
    pal23[0] = (0x01u << 3) | 1u;
    put_le16(&pal23[1], 0x6b5au);
    put_le16(&pal23[3], 0x52b5u);
    put_le16(&pal23[5], 0x294au);
    put_le16(&pal23[7], 0x1084u);
    assert(send_packet(&decoder, pal23) == 1);
    assert(decoder.last_command_id == 0x01u);
    assert(decoder.palette_sequence == 2u);
    assert(decoder.palette_rgb555[0] == 0x6b5au);
    assert(decoder.palette_rgb555[3] == 0x1084u);

    /* Unsupported ATTR_BLK packet is decoded structurally but ignored by V1. */
    uint8_t attr_blk[16] = {0};
    attr_blk[0] = (0x04u << 3) | 1u;
    const uint64_t sequence_before = decoder.palette_sequence;
    assert(send_packet(&decoder, attr_blk) == 0);
    assert(decoder.palette_sequence == sequence_before);

    puts("SGB-lite OK: JOYP packet transport and PALxx global palette recovery");
    return 0;
}
