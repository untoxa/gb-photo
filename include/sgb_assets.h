#ifndef __SGB_ASSETS_INCLUDE__
#define __SGB_ASSETS_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define SGB_BORDER_DEFAULT  0
#define SGB_BORDER_FANCY    1

#define SGB_PALETTE_GRAY    0
#define SGB_PALETTE_RED     1

/** SGB PAL delay

    For SGB on PAL SNES this delay is required on startup,
    otherwise borders don't show up
*/
inline void sgb_pal_delay() {
    for (uint8_t i = 0; i != 4; i++) wait_vbl_done();
}

void sgb_assets_set_palette(uint8_t palette_index) BANKED;
void sgb_assets_set_border(uint8_t border_index) BANKED;

#endif