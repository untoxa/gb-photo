#pragma bank 255

#include "fade.h"
#include "palette.h"

static uint8_t DMGFadeToBlackStep(uint8_t pal, uint8_t step) OLDCALL NAKED {
    pal; step;
__asm
        ldhl    SP, #3
        ld      A, (HL-)
        ld      E, (HL)
        or      A
        ret     Z

        ld      D, A
1$:
        ld      H, #4
2$:
        ld      A, E
        and     #3
        cp      #3
        jr      Z, 3$
        inc     A
3$:
        srl     A
        rr      L
        srl     A
        rr      L

        srl     E
        srl     E

        dec     H
        jr      NZ, 2$

        ld      E, L

        dec     D
        jr      NZ, 1$
        ret
__endasm;
}

static void CGBFadeToBlackStep(const palette_entry_t * pal, uint8_t reg, uint8_t step) OLDCALL NAKED {
    pal; reg; step;
__asm
        ldhl sp, #5
        ld a, (hl-)
        ld b, a

        ld a, (hl-)
        ld c, a
        ld a, #0x80
        ldh (c), a
        inc c

        ld a, (hl-)
        ld l, (hl)
        ld h, a

        ld de, #0x7fff
        ld a, b
        or a
        jr z, 2$
0$:
        res 4, e
        res 1, d
        srl d
        rr e
        res 4, e
        res 1, d
        dec b
        jr nz, 0$
2$:
        ld b, #(8 * 4)
1$:
        ldh a, (_STAT_REG)
        bit STATF_B_BUSY, a
        jr nz, 1$
        ld a, (hl+)
        and e
        ldh (c), a

3$:
        ldh a, (_STAT_REG)
        bit STATF_B_BUSY, a
        jr nz, 3$
        ld a, (hl+)
        and d
        ldh (c), a

        dec b
        jr nz, 1$

        ret
__endasm;
}


void fade_out_modal(void) BANKED {
    for(uint8_t index = 0; index != 5; ++index) {
        if (_cpu == CGB_TYPE) {
            CGBFadeToBlackStep(BkgPalette, BCPS_REG_ADDR, index);
            CGBFadeToBlackStep(SprPalette, OCPS_REG_ADDR, index);
        } else {
            BGP_REG  = DMGFadeToBlackStep(DMG_palette[0], index);
            OBP0_REG = DMGFadeToBlackStep(DMG_palette[1], index);
            OBP1_REG = DMGFadeToBlackStep(DMG_palette[2], index);
        }
        vsync();
        vsync();
    }
}

void fade_in_modal(void) BANKED {
    for(int8_t index = 4; index != -1; --index) {
        if (_cpu == CGB_TYPE) {
            CGBFadeToBlackStep(BkgPalette, BCPS_REG_ADDR, index);
            CGBFadeToBlackStep(SprPalette, OCPS_REG_ADDR, index);
        } else {
            BGP_REG  = DMGFadeToBlackStep(DMG_palette[0], index);
            OBP0_REG = DMGFadeToBlackStep(DMG_palette[1], index);
            OBP1_REG = DMGFadeToBlackStep(DMG_palette[2], index);
        }
        vsync();
        vsync();
    }
}
