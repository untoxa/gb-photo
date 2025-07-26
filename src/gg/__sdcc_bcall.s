        .include "global.s"

        .area   _CODE

        .globl __current_rom
        .globl _rROMB0_MBC5

        .globl ___sdcc_bcall
        .globl ___sdcc_bcall_abc
        .globl ___sdcc_bcall_ehl
;
; trampoline to call banked functions
; used when legacy banking is enabled only
; Usage:
;   call ___sdcc_bcall
;   .dw  <function>
;   .dw  <function_bank>
;
___sdcc_bcall::
        ex      (sp), hl
        ld      c, (hl)
        inc     hl
        ld      b, (hl)
        inc     hl
        ld      a, (hl)
        inc     hl
        inc     hl
        ex      (sp), hl
;
; trampoline to call banked functions with __z88dk_fastcall calling convention
; Usage:
;  ld   a, #<function_bank>
;  ld   bc, #<function>
;  call ___sdcc_bcall_abc
;
___sdcc_bcall_abc::
        push    hl
        ld      l, a
        ld      a, (__current_rom)
        ld      h, a
        ld      a, l
        ld      (__current_rom), a
        ld      (_rROMB0_MBC5), a
        ex      (sp), hl
        inc     sp
        call    ___sdcc_bjump_abc
        dec     sp
        pop     bc
        ld      c, a
        ld      a, b
        ld      (__current_rom), a
        ld      (_rROMB0_MBC5), a
        ld      a, c
        ret
;
___sdcc_bjump_abc:
        push    bc
        ret
;
; default trampoline to call banked functions
; Usage:
;  ld   e, #<function_bank>
;  ld   hl, #<function>
;  call ___sdcc_bcall_ehl
;
___sdcc_bcall_ehl::
        ld      a, (__current_rom)
        push    af
        inc     sp
        ld      a, e
        ld      (__current_rom), a
        ld      (_rROMB0_MBC5), a
        CALL_HL
        dec     sp
        pop     bc
        ld      c, a
        ld      a, b
        ld      (__current_rom), a
        ld      (_rROMB0_MBC5), a
        ld      a, c
        ret
