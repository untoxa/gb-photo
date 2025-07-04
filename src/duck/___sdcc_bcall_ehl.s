        .include        "global.s"

        .globl __current_rom
        .globl _rROMB0_MBC5

        .area _HOME

___sdcc_bcall_ehl::                     ; Performs a long call.
        ldh     a, (__current_rom)
        push    af                      ; Push the current bank onto the stack
        ld      a, e
        ldh     (__current_rom), a
        ld      (_rROMB0_MBC5), a       ; Perform the switch
        rst     0x20
        push    hl
        ldhl    sp, #3
        ld      h, (hl)
        ld      l, a
        ld      a, h
        ldh     (__current_rom), a
        ld      (_rROMB0_MBC5), a
        ld      a, l
        pop     hl
        add     sp, #2
        ret