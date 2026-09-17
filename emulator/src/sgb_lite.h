#ifndef GBCRT_SGB_LITE_H
#define GBCRT_SGB_LITE_H

#include <stdbool.h>
#include <stdint.h>

#define SGB_LITE_PACKET_BYTES 16u
#define SGB_LITE_MAX_PACKETS 7u
#define SGB_LITE_MAX_BYTES (SGB_LITE_PACKET_BYTES * SGB_LITE_MAX_PACKETS)

typedef struct {
    uint8_t command[SGB_LITE_MAX_BYTES];
    unsigned bit_index;
    bool ready_for_pulse;
    bool ready_for_write;
    bool ready_for_stop;

    bool palette_valid;
    uint16_t palette_rgb555[4];
    uint8_t last_command_id;
    uint64_t palette_sequence;
} sgb_lite_decoder_t;

void sgb_lite_reset(sgb_lite_decoder_t *decoder);

/*
 * Feed one value written by the Game Boy to JOYP ($FF00). Only bits 4 and 5
 * are relevant; this is therefore also the logical input expected from a
 * future passive P14/P15 hardware listener.
 *
 * Returns true only when a supported direct palette command completed and the
 * cached global four-color palette changed.
 */
bool sgb_lite_feed_joyp(sgb_lite_decoder_t *decoder, uint8_t joyp_value);

#endif
