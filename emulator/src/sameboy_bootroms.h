#ifndef GBCRT_SAMEBOY_BOOTROMS_H
#define GBCRT_SAMEBOY_BOOTROMS_H

#include <stdint.h>

/*
 * Open-source SameBoy boot ROMs compiled from the project-pinned SameBoy
 * revision 213a12ce93d66b105a113debd9396306066a7cfc using RGBDS v1.0.3.
 *
 * Upstream source: LIJI32/SameBoy BootROMs/dmg_boot.asm and sgb_boot.asm.
 * SameBoy is distributed under the Expat/MIT license; see THIRD_PARTY.md.
 */
extern const uint8_t gbcrt_sameboy_dmg_boot[256];
extern const uint8_t gbcrt_sameboy_sgb_boot[256];

#endif
