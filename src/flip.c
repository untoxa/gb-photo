#pragma bank 255

#include <gbdk/platform.h>
#include <stdbool.h>
#include <string.h>

#include "gbcamera.h"
#include "compat.h"
#include "buffers.h"
#include "flip.h"


static uint8_t hflip_loop;
uint8_t * copy_data_row_flipped_xy(uint8_t * dest, const uint8_t * sour, uint8_t count) NAKED {
    dest; sour; count;
#ifdef NINTENDO
    __asm
        ldhl sp, #2
        ld a, (hl)
        ld l, a
        ld h, #0
        .rept 4
            add hl, hl
        .endm
        dec hl
        dec hl
        add hl, de

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

        pop hl
        inc sp
        jp (hl)
    __endasm;
#else
    __asm
        pop hl
        inc sp
        ex (sp), hl
        ret
    __endasm;
#endif
}

uint8_t * copy_data_row_flipped_x(uint8_t * dest, const uint8_t * sour, uint8_t count) NAKED {
    dest; sour; count;
#ifdef NINTENDO
    __asm
        ldhl sp, #2
        ld a, (hl)
        ld l, a
        dec l
        ld h, #0
        .rept 4
            add hl, hl
        .endm
        add hl, de

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
        .endm
        ld de, #-32
        add hl, de

        ld a, (#_hflip_loop)
        dec a
        jp nz, 1$

        pop hl
        inc sp
        jp (hl)
    __endasm;
#else
    __asm
        pop hl
        inc sp
        ex (sp), hl
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
                sour = copy_data_row_flipped_xy(dest, sour, CAMERA_IMAGE_TILE_WIDTH);
                dest -= (CAMERA_IMAGE_TILE_WIDTH * 16);
            }
            break;
        case camera_flip_x:
            dest = frame_buffer;
            sour = last_seen;
            for (uint8_t i = CAMERA_IMAGE_TILE_HEIGHT; i != 0; i--) {
                sour = copy_data_row_flipped_x(dest, sour, CAMERA_IMAGE_TILE_WIDTH);
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