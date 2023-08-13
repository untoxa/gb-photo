        .include "global.s"

        .title   "vblank sync"
        .module  vsync

        .area	 _HOME

; Pic-n-Rec compatible version of vsync()
_sync_vblank::
        ldh a, (.LCDC)
        and #LCDCF_ON
        ret z
        xor a
        ld hl, #0xFF91          ; address of .vbl_done which is not exposed
        ld (hl), a
1$:
        halt
        nop
        cp (hl)
        jr z, 1$
        ret
