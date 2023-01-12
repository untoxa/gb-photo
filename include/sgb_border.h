#ifndef __SGB_BORDER_H_INCLUDE
#define __SGB_BORDER_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#define SNES_RGB(R,G,B) ((uint16_t)((((B) & 0x1f) << 10) | (((G) & 0x1f) << 5) | (((R) & 0x1f) << 0)))

#define SNES_RED        SNES_RGB(31,  0,  0)
#define SNES_DARKRED    SNES_RGB(15,  0,  0)
#define SNES_GREEN      SNES_RGB( 0, 31,  0)
#define SNES_DARKGREEN  SNES_RGB( 0, 15,  0)
#define SNES_BLUE       SNES_RGB( 0,  0, 31)
#define SNES_DARKBLUE   SNES_RGB( 0,  0, 15)
#define SNES_YELLOW     SNES_RGB(31, 31,  0)
#define SNES_DARKYELLOW SNES_RGB(21, 21,  0)
#define SNES_CYAN       SNES_RGB( 0, 31, 31)
#define SNES_AQUA       SNES_RGB(28,  5, 22)
#define SNES_PINK       SNES_RGB(31,  0, 31)
#define SNES_PURPLE     SNES_RGB(21,  0, 21)
#define SNES_BLACK      SNES_RGB( 0,  0,  0)
#define SNES_DARKGRAY   SNES_RGB(10, 10, 10)
#define SNES_LIGHTGRAY  SNES_RGB(21, 21, 21)
#define SNES_WHITE      SNES_RGB(31, 31, 31)

#define SNES_LIGHTFLESH SNES_RGB(30, 20, 15)
#define SNES_BROWN      SNES_RGB(10, 10,  0)
#define SNES_ORANGE     SNES_RGB(30, 20,  0)
#define SNES_TEAL       SNES_RGB(15, 15,  0)


#define SGB_PKT(P) (((P) << 3) | 1)
#define SNES_LE(C) (uint8_t)(C),(uint8_t)((C) >> 8)

/** sets SGB border

    When using the SGB with a PAL SNES, a delay should be added
    just after program startup such as:

    \code{.c}
    // Wait 4 frames
    // For PAL SNES this delay is required on startup
    for (uint8_t i = 4; i != 0; i--) vsync();
    \endcode
*/
void set_sgb_border(const uint8_t * tiledata, size_t tiledata_size,
                    const uint8_t * tilemap,  size_t tilemap_size,
                    const uint8_t * palette,  size_t palette_size,
                    uint8_t bank) NONBANKED;

#endif