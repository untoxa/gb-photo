#ifndef __SGB_BORDER_H_INCLUDE
#define __SGB_BORDER_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#define SNES_RGB(R,G,B) (uint8_t)((B) << 10 | (G) << 5 | (R)),(uint8_t)(((B) << 10 | (G) << 5 | (R)) >> 8)
#define SNES_COLOR(C) (uint8_t)((C)),(uint8_t)((C) >> 8)

#define SGB_PKT(P) (((P) << 3) | 1)

/** sets SGB border

    When using the SGB with a PAL SNES, a delay should be added
    just after program startup such as:

    \code{.c}
    // Wait 4 frames
    // For PAL SNES this delay is required on startup
    for (uint8_t i = 4; i != 0; i--) wait_vbl_done();
    \endcode
*/
void set_sgb_border(uint8_t * tiledata, size_t tiledata_size,
                    uint8_t * tilemap, size_t tilemap_size,
                    uint8_t * palette, size_t palette_size,
                    uint8_t bank) NONBANKED;

/** SGB PAL delay

    For SGB on PAL SNES this delay is required on startup,
    otherwise borders don't show up
*/
inline void sgb_pal_delay() {
    for (uint8_t i = 0; i != 4; i++) wait_vbl_done();
}

#endif