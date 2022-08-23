#pragma bank 255

#include <gbdk/platform.h>
#include <string.h>

#include "palette.h"
#include "systemdetect.h"
#include "fade_manager.h"

BANKREF(module_palette)

uint8_t DMG_palette[3];
palette_entry_t SprPalette[8];
palette_entry_t BkgPalette[8];

static palette_entry_t cgb_palette[] = {
    CGB_PALETTE(RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK)
};

void palette_cgb_zero(uint8_t reg) OLDCALL BANKED NAKED {
    reg;
__asm
        ldhl sp, #6
        ld c, (hl)
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

uint8_t INIT_module_palette() BANKED {
    if (_is_COLOR) {
        memset(BkgPalette, 0, sizeof(BkgPalette));
        memset(SprPalette, 0, sizeof(SprPalette));
        memcpy(BkgPalette, cgb_palette, sizeof(cgb_palette));
        memcpy(SprPalette, cgb_palette, sizeof(cgb_palette));
    } else {
        DMG_palette[0] = DMG_palette[2] = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);
        DMG_palette[1] = DMG_PALETTE(DMG_WHITE, DMG_WHITE, DMG_DARK_GRAY, DMG_BLACK);
    }
    fade_init();
    if (_is_COLOR) {
        palette_cgb_zero(BCPS_REG_ADDR);
        palette_cgb_zero(OCPS_REG_ADDR);
    }
    fade_setspeed(1);
    return 0;
}