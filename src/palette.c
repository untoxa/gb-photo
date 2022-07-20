#pragma bank 255

#include <gbdk/platform.h>
#include <string.h>

#include "palette.h"
#include "systemdetect.h"

uint8_t DMG_palette[3];
palette_entry_t SprPalette[8];
palette_entry_t BkgPalette[8];

static palette_entry_t cgb_palette[] = {
    CGB_PALETTE(RGB(31, 31, 31), RGB(21, 21, 21), RGB(10, 10, 10), RGB(0, 0, 0))
};

void palette_init() BANKED {
    if (_is_COLOR) {
        memset(BkgPalette, 0, sizeof(BkgPalette));
        memset(SprPalette, 0, sizeof(SprPalette));
        memcpy(BkgPalette, cgb_palette, sizeof(cgb_palette));
        memcpy(SprPalette, cgb_palette, sizeof(cgb_palette));
        return;
    }
    DMG_palette[0] = DMG_palette[2] = DMG_PALETTE(0, 1, 2, 3);
    DMG_palette[1] = DMG_PALETTE(0, 0, 2, 3);
}

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
