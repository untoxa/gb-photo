        .include        "global.s"

        .ez80

        .globl __current_1bpp_colors

        .area   _HOME

; void set_1bpp_data(uint8_t *first_tile, uint16_t nb_tiles, const uint8_t *data) __sdcccall(0) __z88dk_callee __preserves_regs(iyh,iyl);
_set_1bpp_data::
        DISABLE_VBLANK_COPY     ; switch OFF copy shadow SAT

        pop de                  ; pop ret address
        pop hl

        VDP_WRITE_CMD h, l

        ex de, hl               ; hl = ret

        pop bc                  ; bc = ntiles
        ex (sp), hl             ; hl = src

        inc b
        inc c
        push ix

        ld ix, (__current_1bpp_colors)

        push iy
        ld iy, #-4
        add iy, sp
        ld sp, iy
        push bc
        jr 2$

1$:
        ex (sp), hl

        ld d, #8
6$:
        ld c, (hl)
        inc hl

        ld e, #8
5$:
        srl c

        jr c, 10$
        ld a, ixh
        jr 11$
10$:
        ld a, ixl
11$:
        rra
        rr 0 (iy)
        rra
        rr 1 (iy)
        rra
        rr 2 (iy)
        rra
        rr 3 (iy)

        dec e
        jr nz, 5$

        ld a, 0 (iy)
        out (.VDP_DATA), a
        ld a, 1 (iy)
        out (.VDP_DATA), a
        ld a, 2 (iy)
        out (.VDP_DATA), a
        ld a, 3 (iy)
        out (.VDP_DATA), a

        dec d
        jr nz, 6$
2$:
        ex (sp), hl

        dec l
        jr  nz, 1$

        dec h
        jr  nz, 1$

        ld iy, #6
        add iy, sp
        ld sp, iy
        pop iy
        pop ix

        ENABLE_VBLANK_COPY        ; switch ON copy shadow SAT

        ret
