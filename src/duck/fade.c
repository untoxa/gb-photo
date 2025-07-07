#pragma bank 255

#include "fade.h"
#include "palette.h"

static uint8_t DMGFadeToBlackStep(uint8_t pal, uint8_t step) OLDCALL NAKED {
    pal; step;
#if defined(NINTENDO)
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
#endif
}

void fade_out_modal(void) BANKED {
    for(uint8_t index = 0; index != 5; ++index) {
        BGP_REG  = DMGFadeToBlackStep(DMG_palette[0], index);
        OBP0_REG = DMGFadeToBlackStep(DMG_palette[1], index);
        OBP1_REG = DMGFadeToBlackStep(DMG_palette[2], index);
        vsync();
        vsync();
    }
}

void fade_in_modal(void) BANKED {
    for(int8_t index = 4; index != -1; --index) {
        BGP_REG  = DMGFadeToBlackStep(DMG_palette[0], index);
        OBP0_REG = DMGFadeToBlackStep(DMG_palette[1], index);
        OBP1_REG = DMGFadeToBlackStep(DMG_palette[2], index);
        vsync();
        vsync();
    }
}

