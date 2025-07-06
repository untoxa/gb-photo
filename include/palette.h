#ifndef PALETTE_H
#define PALETTE_H

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"

BANKREF_EXTERN(module_palette)

#if defined(NINTENDO)
    #define BCPS_REG_ADDR 0x68
    #define OCPS_REG_ADDR 0x6A
    #define N_PALETTES 8
#elif defined(SEGA)
    #if defined(MASTERSYSTEM)
        #define DMG_BLACK     0x01u
        #define DMG_DARK_GRAY 0x0cu
        #define DMG_LITE_GRAY 0x0eu
        #define DMG_WHITE     0x0fu
    #elif defined(GAMEGEAR)
        #define DMG_BLACK     0x03u
        #define DMG_DARK_GRAY 0x02u
        #define DMG_LITE_GRAY 0x01u
        #define DMG_WHITE     0x00u
    #endif
    #define N_PALETTES 4
#endif

#ifndef DMG_PALETTE
#define DMG_PALETTE(C0, C1, C2, C3) ((uint8_t)((((C3) & 0x03) << 6) | (((C2) & 0x03) << 4) | (((C1) & 0x03) << 2) | ((C0) & 0x03)))
#endif

#define COLOR_PALETTE(C0, C1, C2, C3) {C0, C1, C2, C3}

typedef struct palette_entry_t {
    uint16_t c0, c1, c2, c3;
} palette_entry_t;

extern uint8_t DMG_palette[3];
extern palette_entry_t SprPalette[N_PALETTES];
extern palette_entry_t BkgPalette[N_PALETTES];

inline void palette_apply(void) {
#if defined(NINTENDO)
    set_bkg_palette(0, 8, (palette_color_t *)(&BkgPalette));
    set_sprite_palette(0, 8, (palette_color_t *)(&SprPalette));
#elif defined(SEGA)
    set_bkg_palette(0, 1, (palette_color_t *)(&BkgPalette));
    set_sprite_palette(0, 1, (palette_color_t *)(&SprPalette));
#endif
}

void palette_reload(void) BANKED;
uint8_t INIT_module_palette(void) BANKED;

#endif
