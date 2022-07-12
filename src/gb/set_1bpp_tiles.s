        .include        "global.s"

        .globl __current_1bpp_colors

        .area   _HOME

; void set_1bpp_data(uint8_t *first_tile, uint8_t nb_tiles, const uint8_t *data) OLDCALL PRESERVES_REGS(b, c);
_set_1bpp_data::
        push bc

        lda hl, 8(sp)
        ld a, (hl-) ; ID of 1st tile
        ld d, a
        ld a, (hl-) ; ID of 1st tile
        ld e, a
        ld a, (hl-) ; Nb of tiles
        ld c, a
        ld a, (hl-) ; Src ptr
        ld l, (hl)
        ld h, a
        
1$:
        ; Wrap from past $97FF to $8800 onwards
        ; This can be reduced to "bit 4 must be clear if bit 3 is set"
        bit 3, h
        jr z, 2$
        res 4, h
2$:
        ld b, #8
3$:
        push bc
        ld a, (de)     ; a == bits
        inc de
        push de
        push hl

        ld hl, #__current_1bpp_colors
        ld c, (hl)
        inc hl
        ld h, (hl)
        ld l, c         ; L ==.fg_colour, H == .bg_colour

        ld e, #8
8$:
        rra
        jr c, 7$
        ld d, h
        jr 9$
7$:
        ld d, l
9$:
        srl d
        rr c
        srl d
        rr b

        dec e
        jr nz, 8$

        pop hl

        WAIT_STAT
        ld (hl), c
        inc l
        ld (hl), b
        inc hl

        pop de
        pop bc

        dec b
        jr nz, 3$

        dec c
        jr nz, 1$

        pop bc
        ret