        .include        "global.s"

        .globl _vwf_current_rotate, _vwf_current_mask, _vwf_inverse_map, _vwf_tile_data, _vwf_inverse_map

        .area _DATA

__save:
        .ds 0x01

        .area _CODE

; void vwf_memcpy(void* to, const void* from, size_t n, UBYTE bank);
_vwf_memcpy::
        ldh a, (__current_bank)
        ld  (#__save),a

        ldhl sp, #8
        ld  a, (hl-)
        ldh (__current_bank),a
        ld  (rROMB0), a

        ld a, (hl-)
        ld b, a
        ld a, (hl-)
        ld c, a
        ld a, (hl-)
        ld d, a
        ld a, (hl-)
        ld e, a
        ld a, (hl-)
        ld l, (hl)
        ld h, a

        inc b
        inc c
        jr 2$
1$:
        ld a, (de)
        ld (hl+), a
        inc de
2$:
        dec c
        jr nz, 1$
        dec b
        jr nz, 1$

        ld  a, (#__save)
        ldh (__current_bank),a
        ld  (rROMB0), a
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

; void vwf_print_shift_char(void * dest, const void * src, UBYTE bank);
_vwf_print_shift_char::
        ldhl sp, #2

        ldh a, (__current_bank)
        push af
        ld a, (hl)
        ldh (__current_bank), a
        ld  (rROMB0), a

        ld h, d
        ld l, e
        ld d, b
        ld e, c

        ld b, #8
3$:
        ld a, (de)
        ld c, a
        ld a, (_vwf_inverse_map)
        xor c
        ld c, a
        inc de

        ld a, (_vwf_current_rotate)
        sla a
        jr z, 1$
        jr c, 4$
        srl a
        srl a
        jr nc, 6$
        srl c
6$:
        or a
        jr z, 1$
2$:
        srl c
        srl c
        dec a
        jr nz, 2$
        jr 1$
4$:
        srl a
        srl a
        jr nc, 7$
        sla c
7$:     or a
        jr z, 1$
5$:
        sla c
        sla c
        dec a
        jr nz, 5$
1$:
        ld a, (_vwf_current_mask)
        and (hl)
        or c
        ld (hl+), a

        dec b
        jr nz, 3$

        pop af
        ldh (__current_bank),a
        ld  (rROMB0), a

        pop hl
        inc sp
        jp (hl)

_vwf_swap_tiles::
        ld      hl, #_vwf_tile_data
        ld      de, #(_vwf_tile_data + 8)
        .rept 8
                ld      a, (de)
                inc     de
                ld      (hl+), a
        .endm
        ld      a, (_vwf_inverse_map)
        .rept 7
        ld      (hl+), a
        .endm
        ld      (hl), a
        ret
