#include <gbdk/platform.h>
#include <stdint.h>

uint8_t sio_exchange_byte(uint8_t data) NAKED {
    data;
    __asm
        ld e, #0b00000001
        ld c, #_GG_EXT_CTL
        out (c), e
        ld c, #_GG_EXT_7BIT
        out (c), e
        ld b, #8
1$:
        rlca
        rl e
        rl e
        res 7, e
        set 2, e
        out (c), e
        res 2, e
        out (c), e
        in e, (c)
        rrca
        srl e
        rla
        djnz 1$
        ret
    __endasm;
}
