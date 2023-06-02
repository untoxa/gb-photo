#pragma bank 255

#include <gb/isr.h>
#include "remote.h"

#define WATCHDOG_DELAY 2

BANKREF(module_remote)

volatile uint8_t remote_keys;
volatile uint8_t remote_watchdog;

inline void SIO_request_transfer(void) {
    SC_REG = 0x80;      // start transfer with external clock
}
inline void SIO_cancel_transfer(void) {
    SC_REG = 0x00;      // reset transfer request
}

void isr_remote_SIO(void) NONBANKED NAKED {
__asm
        push af
        push hl
        push de

        ld hl, #_remote_keys
        ldh a, (_SB_REG)    ; 0bS0IPXXXX S-stop, I-identifier, P-parity, XXXX - 4 button bits
        ld e, a

        ld a, #0x80
        ldh (_SC_REG), a    ; initialize next transfer
        ld a, e

        and #0xC0
        cp #0x80
        jr nz, 1$           ; check bit 7 is 1 and bit 6 is 0

        ld a, e
        ld d, #0
        xor a
        .rept 4
            rrc e
            adc d
        .endm
        xor e
        and #1
        jr nz, 1$           ; check parity bit (result must be 0 when parity matches)

        bit 1, e
        jr nz, 2$

        ld a, #0xf0
        and e
        swap a
        ld e, a
        ld a, #0xf0
        jr 3$
2$:
        ld a, #0xf0
        and e
        ld e, a
        ld a, #0x0f
3$:
        and (hl)
        or e
        ld (hl), a

        jr 4$
1$:
        xor a
        ldh (_SC_REG), a    ; reset transfer
        ld (hl), a
        ld (_remote_watchdog), a

4$:
        pop de
        pop hl
5$:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr nz, 5$

        pop af
        reti
__endasm;
}
ISR_VECTOR(VECTOR_SERIAL, isr_remote_SIO)

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
        set_interrupts(IE_REG | SIO_IFLAG);
        SIO_cancel_transfer();
        SIO_request_transfer();
    } else {
        set_interrupts(IE_REG & ~SIO_IFLAG);
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