#include <gbdk/platform.h>
#include <stdint.h>

#include "sfxplayer.h"

#define PERIOD_8192 0x700
#define LENGTH_8192 0xfe
#define PERIOD_1920 0x3BB
#define LENGTH_1920 0x3b

#define CH3_LENGTH (LENGTH_8192)
#define CH3_VALUES (PERIOD_8192 | 0xc000)


volatile UINT8 sfx_play_bank = SFX_STOP_BANK;
const UINT8 * sfx_play_sample = NULL;
UINT8 sfx_frame_skip;

UINT8 sfx_play_isr(void) NONBANKED NAKED {
#if defined(__SDCC)
#if defined(NINTENDO)
__asm
.macro copy_reg ?lbl
        sla b
        jr nc, lbl
        ld a, (hl+)
        ldh (c), a
lbl:
        inc c
.endm

        ld hl, #_sfx_play_sample
        ld a, (hl+)
        ld e, a
        or (hl)
        ret z                       ; return FALSE
        ld d, (hl)

        ld hl, #_sfx_frame_skip
        xor a
        or (hl)
        jr z, 7$
        dec (hl)
        ret                         ; A != 0 that returns TRUE
7$:
        ld h, d
        ld l, e                     ; HL = current position inside the sample

        ldh a, (__current_rom)      ; save bank and switch
        ld e, a
        ld a, (_sfx_play_bank)
        inc a                       ; SFX_STOP_BANK ?
        ret z                       ; return FALSE
        dec a
        ldh (__current_rom), a
        ld (_rROMB0_MBC5), a

        ld d, #0x0f
        ld a, (hl)
        swap a
        and d
        ld (_sfx_frame_skip), a

        ld a, (hl+)
        and d
        ld d, a                     ; d = frame channel count
        jp z, 6$
2$:
        ld a, (hl+)
        ld b, a                     ; a = b = channel no + register mask

        and #0b00000111
        cp #5
        jr c, 3$
        cp #7
        jr z, 5$                    ; terminator

        ldh a, (_NR51_REG)
        ld c, a
        and #0b10111011
        ldh (_NR51_REG), a

        xor a
        ld (_NR30_REG), a

        .irp ofs,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
            ld a, (hl+)
            ldh (__AUD3WAVERAM+ofs), a
        .endm

        ld a, b
        cp #6
        jr nz, 9$                   ; just load waveform, not play

        ld a, #0x80
        ldh (_NR30_REG),a
        ld a, #(CH3_LENGTH)         ; length of wave
        ldh (_NR31_REG),a
        ld a, #0x20                 ; volume
        ldh (_NR32_REG),a
        ld a, #<(CH3_VALUES)
        ldh (_NR33_REG),a
        ld a, #>(CH3_VALUES)
        ldh (_NR34_REG),a

9$:
        ld a, c
        ldh (_NR51_REG), a

        jr 4$
5$:                                 ; terminator
        ld hl, #0
        ld d, l
        jr 0$
3$:
        ld  c, a
        add a
        add a
        add c
        add #_NR10_REG
        ld c, a                     ; c = NR10_REG + (a & 7) * 5

        .rept 5
            copy_reg
        .endm

4$:
        dec d
        jp nz, 2$
6$:
        inc d                       ; return TRUE if still playing
0$:
        ld a, l                     ; save current position
        ld (_sfx_play_sample), a
        ld a, h
        ld (_sfx_play_sample + 1), a

        ld a, e                     ; restore bank
        ldh (__current_rom), a
        ld (_rROMB0_MBC5), a

        ld a, d                     ; result in a

        ret
__endasm;
#else
__asm
        xor a
        ld l, a
        ret
__endasm;
#endif
#endif
}
