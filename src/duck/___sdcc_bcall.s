        .include        "global.s"

        .area _HOME

;; !!! ONLY WORKS WITH __sdcccall(0) calling convention

___sdcc_bcall::
banked_call::                   ; Performs a long call.
        pop     hl              ; Get the return address
        ldh     a, (__current_bank)
        push    af              ; Push the current bank onto the stack
        ld      a, (hl+)        ; Fetch the call address
        ld      e, a
        ld      a, (hl+)
        ld      d, a
        ld      a, (hl+)        ; ...and page
        inc     hl              ; Yes this should be here
        push    hl              ; Push the real return address
        ldh     (__current_bank), a
        ld      (_rROMB0_MBC5), a
        ld      l, e
        ld      h, d
        rst     0x20
banked_ret::
        pop     bc              ; Pop return address
        pop     af              ; Pop the old bank
        ldh     (__current_bank), a
        ld      (_rROMB0_MBC5), a
        push    bc
        ret
