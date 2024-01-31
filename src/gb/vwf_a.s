        .include        "global.s"

        .globl _vwf_current_rotate, _vwf_tile_data

        .area _DATA

__save:
        .ds 0x01
__counter:
        .ds 0x01

        .area _CODE

; void vwf_memcpy(void* to, const void* from, size_t n, UBYTE bank);
_vwf_memcpy::
        ldh a, (__current_bank)
        ld  (#__save),a

        ldhl sp, #4
        ld a, (hl-)

        ldh (__current_bank),a
        ld  (rROMB0), a

        ld a, (hl-)
        ld l, (hl)
        ld h, a

        ld a, b
        ld b, h
        ld h, a
        ld a, c
        ld c, l
        ld l, a

        inc b
        inc c
        jr 2$
1$:
        ld a, (hl+)
        ld (de), a
        inc de
2$:
        dec c
        jr nz, 1$
        dec b
        jr nz, 1$

        ld  a, (#__save)
        ldh (__current_bank),a
        ld  (rROMB0), a

        pop hl
        add sp, #3
        ret

; UBYTE vwf_read_banked_ubyte(const void * src, UBYTE bank);
_vwf_read_banked_ubyte::
        ld  hl, #__current_bank
        ld  c, (hl)

        ld  (hl), a
        ld  (rROMB0), a

        ld  a, (de)
        ld  e, a

        ld  a, c
        ld  (hl), a
        ld  (rROMB0), a

        ld a, e
        ret

; void vwf_print_shift_char_right(void * dest, const void * src, UBYTE bank);
_vwf_print_shift_char_right::
        ldhl sp, #2

        ldh a, (__current_bank)
        push af
        ld a, (hl)
        ldh (__current_bank), a
        ld  (rROMB0), a

        ld hl, #__counter
        ld (hl), #8
11$:
        ld hl, #10$
        ld a, (_vwf_current_rotate)
        add a
        sub l
        cpl
        inc a
        ld l, a
        jr c, 12$
        dec h
12$:
        ld a, (bc)
        inc bc
        jp (hl)

        .rept 7
            srl a
        .endm
10$:
        ld h, d
        ld l, e
        xor (hl)
        ld (hl+), a
        ld d, h
        ld e, l

        ld hl, #__counter
        dec (hl)
        jr nz, 11$

        pop af
        ldh (__current_bank),a
        ld  (rROMB0), a

        pop hl
        inc sp
        jp (hl)

; void vwf_print_shift_char_left(void * dest, const void * src, UBYTE bank);
_vwf_print_shift_char_left::
        ldhl sp, #2

        ldh a, (__current_bank)
        push af
        ld a, (hl)
        ldh (__current_bank), a
        ld  (rROMB0), a

        ld hl, #__counter
        ld (hl), #8
11$:
        ld hl, #10$
        ld a, (_vwf_current_rotate)
        add a
        sub l
        cpl
        inc a
        ld l, a
        jr c, 12$
        dec h
12$:
        ld a, (bc)
        inc bc
        jp (hl)

        .rept 7
            sla a
        .endm
10$:
        ld h, d
        ld l, e
        xor (hl)
        ld (hl+), a
        ld d, h
        ld e, l

        ld hl, #__counter
        dec (hl)
        jr nz, 11$

        pop af
        ldh (__current_bank),a
        ld  (rROMB0), a

        pop hl
        inc sp
        jp (hl)


_vwf_swap_tiles::
        ld hl, #_vwf_tile_data
        ld de, #(_vwf_tile_data + 8)
        .rept 8
            ld a, (de)
            inc de
            ld (hl+), a
        .endm
        xor a
        .rept 7
            ld (hl+), a
        .endm
        ld (hl), a
        ret
