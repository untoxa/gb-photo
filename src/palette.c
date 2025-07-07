#pragma bank 255

#include <gbdk/platform.h>
#include <string.h>

#include "palette.h"
#include "systemdetect.h"
#include "fade.h"

#include "state_camera.h"

BANKREF(module_palette)

uint8_t DMG_palette[3];
palette_entry_t SprPalette[N_PALETTES];
palette_entry_t BkgPalette[N_PALETTES];

static palette_entry_t cgb_palette[] = {
    COLOR_PALETTE(RGB_WHITE,         RGB_LIGHTGRAY,     RGB_DARKGRAY,      RGB_BLACK       ),  // Arctic
    COLOR_PALETTE(RGB_WHITE,         RGB8(169,243,255), RGB8(45,170,255),  RGB8(0,66,128)  ),  // Cyanotype
    COLOR_PALETTE(RGB8(255,251,185), RGB8(255,212,0),   RGB8(239,0,142),   RGB8(38,6,139)  ),  // Thermal
    COLOR_PALETTE(RGB8(255,242,213), RGB8(197,185,152), RGB8(0,132,143),   RGB_BLACK       ),  // Circuits
    COLOR_PALETTE(RGB8(248,196,254), RGB8(193,93,228),  RGB8(62,0,143),    RGB_BLACK       ),  // Grapescale
    COLOR_PALETTE(RGB8(240,240,240), RGB8(218,196,106), RGB8(112,88,52),   RGB8(30,30,30)  ),  // Pocket Camera JAP
    COLOR_PALETTE(RGB8(254,218,27),  RGB8(223,121,37),  RGB8(182,0,119),   RGB8(56,41,119) )   // My Friend from Bavaria
};

#if defined(NINTENDO)
void palette_cgb_zero(uint8_t reg) NAKED {
    reg;
__asm
        ld c, a
        ld a, #0x80
        ldh (c), a
        inc c

        ld b, #(8 * 4 * 2)
1$:
        ldh a, (_STAT_REG)
        bit STATF_B_BUSY, a
        jr nz, 1$
        xor a
        ldh (c), a

        dec b
        jr nz, 1$

        ret
__endasm;
}
#endif

void palette_reload(void) BANKED {
    if (_is_COLOR) {
        memset(BkgPalette, 0, sizeof(BkgPalette));
        memset(SprPalette, 0, sizeof(SprPalette));

        const palette_entry_t * pal = cgb_palette + (OPTION(cgb_palette_idx) % LENGTH(cgb_palette));
        memcpy(BkgPalette, pal, sizeof(palette_entry_t));
        memcpy(&SprPalette[0].c1, pal, sizeof(palette_entry_t) - sizeof(SprPalette[0].c0));
#if defined(NINTENDO)
        SprPalette[1].c2 = pal->c0; SprPalette[1].c3 = pal->c2;
#endif
    } else {
        DMG_palette[0] = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);
        DMG_palette[1] = DMG_PALETTE(DMG_BLACK, DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY);
        DMG_palette[2] = DMG_PALETTE(DMG_BLACK, DMG_BLACK, DMG_WHITE, DMG_DARK_GRAY);
    }
}

uint8_t INIT_module_palette(void) BANKED {
    palette_reload();
#if defined(NINTENDO)
    BGP_REG = DMG_PALETTE(DMG_BLACK, DMG_BLACK, DMG_BLACK, DMG_BLACK);
    OBP0_REG = DMG_PALETTE(DMG_BLACK, DMG_BLACK, DMG_BLACK, DMG_BLACK);
    OBP1_REG = DMG_PALETTE(DMG_BLACK, DMG_BLACK, DMG_BLACK, DMG_BLACK);
#endif
    if (_is_COLOR) {
#if defined(NINTENDO)
        palette_cgb_zero(BCPS_REG_ADDR);
        palette_cgb_zero(OCPS_REG_ADDR);
#elif defined(SEGA)
        for (uint8_t i = 0; i != 16; i++) {
            set_palette_entry(0, i, 0);
            set_palette_entry(1, i, 0);
        }
#endif
    }
    return 0;
}