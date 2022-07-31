#include <gbdk/platform.h>
#include <stdint.h>

int8_t log2(uint16_t v) NAKED {
    v;
    __asm
#ifdef NINTENDO
        ld a, #15
        scf
        rl e
        rl d
        ret c
1$:
        add hl, hl
        dec a
        jr nc, 1$
        ret
#else
        ld a, #16
        scf
1$:
        adc hl, hl
        dec a
        jr nc, 1$
        ret
#endif
    __endasm;
}