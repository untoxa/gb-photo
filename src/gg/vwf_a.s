        .include        "global.s"

        .globl _vwf_current_rotate, _vwf_current_mask, _vwf_inverse_map, _vwf_tile_data, _vwf_inverse_map

        .ez80

        .area _DATA

__save:
        .ds 0x01

        .area _CODE

; void vwf_memcpy(void* to, const void* from, size_t n, UBYTE bank)  __z88dk_callee;
_vwf_memcpy::
        pop iy
        pop de
        pop hl
        pop bc
        dec sp
        ex (sp), iy

        ld  a, (.MAP_FRAME1)
        ld  (#__save), a
        ld  a, iyh
        ld (.MAP_FRAME1), a

        ldir

        ld  a, (#__save)
        ld  (.MAP_FRAME1),a
        ret

; UBYTE vwf_read_banked_ubyte(const void * src, UBYTE bank) __z88dk_callee;
_vwf_read_banked_ubyte::
        ld  a, (.MAP_FRAME1)
        ld  (#__save), a

        pop hl
        pop de
        dec sp
        ex (sp), hl

        ld a, h
        ld (.MAP_FRAME1), a

        ex de, hl
        ld l, (hl)

        ld  a, (#__save)
        ld (.MAP_FRAME1), a
        ret

; void vwf_print_shift_char(void * dest, const void * src, UBYTE bank);
_vwf_print_shift_char::
        ld  a, (.MAP_FRAME1)
        ld  (#__save), a

        pop hl
        pop de
        pop bc
        dec sp
        ex (sp), hl

        ld a, h
        ld (.MAP_FRAME1), a

        ex de, hl
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
        ld (hl), a
        inc hl

        dec b
        jr nz, 3$

        ld  a, (#__save)
        ld (.MAP_FRAME1), a

        ret

_vwf_swap_tiles::
        ld      de, #_vwf_tile_data
        ld      hl, #(_vwf_tile_data + 8)
        .rept 8
                ldi
        .endm
        ld      a, (_vwf_inverse_map)
        ld      (de), a
        ld      h, d
        ld      l, e
        inc     de
        .rept 7
                ldi
        .endm
        ret
