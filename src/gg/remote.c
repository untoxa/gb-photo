#pragma bank 255

#include <gb/isr.h>
#include "compat.h"
#include "remote.h"

#define WATCHDOG_DELAY 2

BANKREF(module_remote)

volatile uint8_t remote_keys;
volatile uint8_t remote_watchdog;

volatile uint8_t sio_counter = 0;
volatile uint8_t sio_value = 0;

inline void SIO_request_transfer(void) {
    sio_counter = sio_value = 0;
    GG_EXT_7BIT = 0xff;
    GG_EXT_CTL = LINK_SLAVE;                // start transfer with external clock
}
inline void SIO_cancel_transfer(void) {
    GG_EXT_CTL = LINK_SLAVE | GGEXT_NINT;   // reset transfer request
}

void NMI_ISR(void) NONBANKED NAKED {
__asm
        push af
        push hl
        push de

        ld hl, #_sio_value
        in a, (_GG_EXT_7BIT)
        rrca
        rl (hl)
        rla
        rla
        and #LINK_MOSI
        out (_GG_EXT_7BIT), a
        ld hl, #_sio_counter
        inc (hl)
        ld a, #7
        and (hl)
        ld (hl), a
        jp nz, 4$

        ld hl, #_remote_keys
        ld a, (_sio_value)  ; 0bS0IPXXXX S-stop, I-identifier, P-parity, XXXX - 4 button bits
        ld e, a

        and #0xC0
        cp #0x80
        jp nz, 1$           ; check bit 7 is 1 and bit 6 is 0

        ld a, e
        ld d, #0
        xor a
        .rept 4
            rrc e
            adc d
        .endm
        xor e
        and #1
        jp nz, 1$           ; check parity bit (result must be 0 when parity matches)

        bit 1, e
        jp nz, 2$

        ld a, #0xf0
        and e
        .rept 4
            rlca
        .endm
        ld e, a
        ld a, #0xf0
        jp 3$
2$:
        ld a, #0xf0
        and e
        ld e, a
        ld a, #0x0f
3$:
        and (hl)
        or e
        ld (hl), a

        jp 4$
1$:
        xor a
        ld (_sio_counter), a
        ld (hl), a
        ld (_remote_watchdog), a

4$:
        ld a, #(LINK_SLAVE | GGEXT_NINT)
        out (_GG_EXT_CTL), a
        ld a, #LINK_SLAVE
        out (_GG_EXT_CTL), a

        pop de
        pop hl
        pop af
        retn
__endasm;
}

void isr_remote_VBL(void) NONBANKED {
    if (remote_watchdog < WATCHDOG_DELAY) {
        if ((++remote_watchdog) == WATCHDOG_DELAY) {
            SIO_cancel_transfer();
            SIO_request_transfer();
        }
    }
}

uint8_t remote_activate(uint8_t value) BANKED {
    if (value) {
        SIO_cancel_transfer();
        SIO_request_transfer();
    } else {
        SIO_cancel_transfer();
    }
    remote_keys = 0;
    return value;
}

uint8_t INIT_module_remote(void) BANKED {
    CRITICAL {
        // reinstall VBL handler (watchdog)
        remove_VBL(isr_remote_VBL);
        add_VBL(isr_remote_VBL);

        remote_watchdog = 0;
        remote_keys = 0;
    }
    remote_activate(REMOTE_ENABLED);
    return 0;
}