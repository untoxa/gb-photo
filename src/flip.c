#pragma bank 255

#include <gbdk/platform.h>
#include <stdbool.h>
#include <string.h>

#include "gbcamera.h"
#include "compat.h"
#include "buffers.h"
#include "flip.h"

BANKREF(module_flip)

static uint8_t hflip_loop;
uint8_t * copy_data_row_flipped_xy(uint8_t * dest, const uint8_t * sour) NAKED {
    dest; sour;
#ifdef NINTENDO
    __asm
        ld hl, #((CAMERA_IMAGE_TILE_WIDTH << 4) - 2)
        add hl, de
        ld a, #CAMERA_IMAGE_TILE_WIDTH
1$:
        ld (#_hflip_loop), a

        .rept 8
            ld d, #>_flip_recode_table
            .rept 2
                ld a, (bc)
                inc bc
                ld e, a
                ld a, (de)
                ld (hl+), a
            .endm
            ld de, #-4
            add hl, de
        .endm

        ld a, (#_hflip_loop)
        dec a
        jp nz, 1$

        ret
    __endasm;
#else
    __asm
        .ez80

        push ix

        ld ixh, d
        ld ixl, e

        ex de, hl
        ld hl, #((CAMERA_IMAGE_TILE_WIDTH - 1) << 4)
        add hl, de
        ex de, hl
        ld iyh, d
        ld iyl, e

        ld hl, #_hflip_loop
        ld (hl), #CAMERA_IMAGE_TILE_WIDTH
        ld d, #>_flip_recode_table
1$:
        .irp idy, 0,2,4,6,8,10,12,14
            .irp idx, 0,1
                ld e, 14-idy+idx(ix)
                ld a, (de)
                ld idy+idx(iy), a
            .endm
        .endm

        ld bc, #CAMERA_IMAGE_TILE_WIDTH
        add ix, bc
        ld bc, #-CAMERA_IMAGE_TILE_WIDTH
        add iy, bc

        dec (hl)
        jp nz, 1$

        ld d, ixh
        ld e, ixl

        pop ix
        ret
    __endasm;
#endif
}

uint8_t * copy_data_row_flipped_x(uint8_t * dest, const uint8_t * sour) NAKED {
    dest; sour;
#ifdef NINTENDO
    __asm
        ld hl, #((CAMERA_IMAGE_TILE_WIDTH - 1) << 4)
        add hl, de
        ld a, #CAMERA_IMAGE_TILE_WIDTH
1$:
        ld (#_hflip_loop), a

        ld d, #>_flip_recode_table
        .rept 16
            ld a, (bc)
            inc bc
            ld e, a
            ld a, (de)
            ld (hl+), a
        .endm
        ld de, #-32
        add hl, de

        ld a, (#_hflip_loop)
        dec a
        jp nz, 1$

        ret
    __endasm;
#else
    __asm
        .ez80

        push ix

        ld ixh, d
        ld ixl, e

        ex de, hl
        ld hl, #((CAMERA_IMAGE_TILE_WIDTH - 1) << 4)
        add hl, de
        ex de, hl
        ld iyh, d
        ld iyl, e

        ld hl, #_hflip_loop
        ld (hl), #CAMERA_IMAGE_TILE_WIDTH
        ld d, #>_flip_recode_table
1$:
        .irp idx, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
            ld e, idx(ix)
            ld a, (de)
            ld idx(iy), a
        .endm

        ld bc, #CAMERA_IMAGE_TILE_WIDTH
        add ix, bc
        ld bc, #-CAMERA_IMAGE_TILE_WIDTH
        add iy, bc

        dec (hl)
        jp nz, 1$

        ld d, ixh
        ld e, ixl

        pop ix
        ret
    __endasm;
#endif
}

uint8_t * get_flipped_last_seen_image(camera_flip_e flip, bool copy) BANKED {
    static uint8_t * dest, * sour;
    CAMERA_SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    switch (flip) {
        case camera_flip_xy:
            dest = frame_buffer + (CAMERA_IMAGE_TILE_HEIGHT - 1) * (CAMERA_IMAGE_TILE_WIDTH * 16);
            sour = last_seen;
            for (uint8_t i = CAMERA_IMAGE_TILE_HEIGHT; i != 0; i--) {
                sour = copy_data_row_flipped_xy(dest, sour);
                dest -= (CAMERA_IMAGE_TILE_WIDTH * 16);
            }
            break;
        case camera_flip_x:
            dest = frame_buffer;
            sour = last_seen;
            for (uint8_t i = CAMERA_IMAGE_TILE_HEIGHT; i != 0; i--) {
                sour = copy_data_row_flipped_x(dest, sour);
                dest += (CAMERA_IMAGE_TILE_WIDTH * 16);
            }
            break;
        default:
            if (!copy) {
                return last_seen;
            }
            memcpy(frame_buffer, last_seen, CAMERA_IMAGE_SIZE);
            break;
    }
    return frame_buffer;
}

uint8_t INIT_module_flip(void) BANKED {
    memset(frame_buffer, 0, sizeof(frame_buffer));
    return 0;
}
