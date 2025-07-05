#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "systemdetect.h"
#include "fade_manager.h"
#include "palette.h"

uint8_t fade_running;
uint8_t fade_frames_per_step;
uint8_t fade_timer;
uint8_t fade_style = TRUE;

static const uint8_t fade_speeds[] = {0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F};

static uint8_t fade_frame;
static FADE_DIRECTION fade_direction;

static void CGBFadeToWhiteStep(const palette_entry_t * pal, uint8_t reg, uint8_t step) OLDCALL NAKED {
    pal; reg; step;
#if defined(NINTENDO)
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

        ld de, #0x0000
        ld a, b
        or a
        jr z, 2$
0$:
        sla e
        rl d
        set 0, e
        set 5, e
        set 2, d
        dec b
        jr nz, 0$
2$:
        ld b, #(8 * 4)
1$:
        ldh a, (_STAT_REG)
        bit STATF_B_BUSY, a
        jr nz, 1$
        ld a, (hl+)
        or e
        ldh (c), a

3$:
        ldh a, (_STAT_REG)
        bit STATF_B_BUSY, a
        jr nz, 3$
        ld a, (hl+)
        or d
        ldh (c), a

        dec b
        jr nz, 1$

        ret
__endasm;
#elif defined(SEGA)
__asm
        ret
__endasm;
#endif
}

static void CGBFadeToBlackStep(const palette_entry_t * pal, uint8_t reg, uint8_t step) OLDCALL NAKED {
    pal; reg; step;
#if defined(NINTENDO)
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
#elif defined(SEGA)
__asm
        ret
__endasm;
#endif
}

void fade_apply_palette_change_color(uint8_t index) BANKED {
    if (fade_style) {
        CGBFadeToBlackStep(BkgPalette, BCPS_REG_ADDR, index);
        CGBFadeToBlackStep(SprPalette, OCPS_REG_ADDR, index);
    } else {
        CGBFadeToWhiteStep(BkgPalette, BCPS_REG_ADDR, index);
        CGBFadeToWhiteStep(SprPalette, OCPS_REG_ADDR, index);
    }
}

static uint8_t DMGFadeToWhiteStep(uint8_t pal, uint8_t step) OLDCALL NAKED {
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
        jr      Z, 3$
        dec     A
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

void fade_apply_palette_change_dmg(uint8_t index) BANKED {
    index;
#if defined(NINTENDO)
    if (index > 4) index = 4;
    if (!fade_style) {
        BGP_REG = DMGFadeToWhiteStep(DMG_palette[0], index);
        OBP0_REG = DMGFadeToWhiteStep(DMG_palette[1], index);
        OBP1_REG = DMGFadeToWhiteStep(DMG_palette[2], index);
    } else {
        BGP_REG = DMGFadeToBlackStep(DMG_palette[0], index);
        OBP0_REG = DMGFadeToBlackStep(DMG_palette[1], index);
        OBP1_REG = DMGFadeToBlackStep(DMG_palette[2], index);
    }
#endif
}

void fade_init(void) BANKED {
    fade_frames_per_step = fade_speeds[2];
    fade_timer = FADED_OUT_FRAME;
    fade_running = FALSE;
    if (_is_COLOR) {
        fade_apply_palette_change_color(fade_timer);
        return;
    }
    fade_apply_palette_change_dmg(FADED_OUT_FRAME);
}

static void fade_in(void) {
    if (fade_timer == FADED_IN_FRAME) {
        return;
    }
    fade_frame = 0;
    fade_direction = FADE_IN;
    fade_running = TRUE;
    fade_timer = FADED_OUT_FRAME;
    if (_is_COLOR) {
        fade_apply_palette_change_color(FADED_OUT_FRAME);
        return;
    }
    fade_apply_palette_change_dmg(FADED_OUT_FRAME);
}

static void fade_out(void) {
    if (fade_timer == FADED_OUT_FRAME) {
        return;
    }
    fade_frame = 0;
    fade_direction = FADE_OUT;
    fade_running = TRUE;
    fade_timer = FADED_IN_FRAME;
    if (_is_COLOR) {
        fade_apply_palette_change_color(fade_timer);
        return;
    }
    fade_apply_palette_change_dmg(FADED_IN_FRAME);
}

static void fade_update(void) {
    if (fade_running) {
        if ((fade_frame++ & fade_frames_per_step) == 0) {
            if (fade_direction == FADE_IN) {
                if (fade_timer > FADED_IN_FRAME) fade_timer--;
                if (fade_timer == FADED_IN_FRAME) fade_running = FALSE;
            } else {
                if (fade_timer < FADED_OUT_FRAME) fade_timer++;
                if (fade_timer == FADED_OUT_FRAME) fade_running = FALSE;
            }
            if (_is_COLOR) {
                fade_apply_palette_change_color(fade_timer);
                return;
            }
            fade_apply_palette_change_dmg(fade_timer);
        }
    }
}

void fade_setspeed(uint8_t speed) BANKED {
    fade_frames_per_step = fade_speeds[speed];
}

inline uint8_t fade_isfading(void) {
  return fade_running;
}

void fade_in_modal(void) BANKED {
    fade_in();
    while (fade_isfading()) {
        sync_vblank();
        fade_update();
    }
}

void fade_out_modal(void) BANKED {
    fade_out();
    while (fade_isfading()) {
        sync_vblank();
        fade_update();
    }
}