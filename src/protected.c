#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "gbcamera.h"
#include "vector.h"

#include "protected.h"

void protected_pack(uint8_t * v) BANKED {
    uint8_t i, elem;
    VECTOR_ITERATE(v, i, elem) {
        protected_modify_slot(elem, i);
    }
}

uint16_t protected_scale_line_part(const void * ptr) NAKED {
    ptr;
#ifdef NINTENDO
    __asm
.macro .SHIFT_6_2 reg
        rla
        rla
        rl reg
        rla
        rla
        rla
        rla
        rl reg
.endm
.macro .SCALE regh regl
        ld a, (hl+)
        .SHIFT_6_2 regl
        ld a, (hl-)
        .SHIFT_6_2 regh
.endm
.macro  .ADD_A_HL
        add l
        ld l, a
        adc h
        sub l
        ld h, a
.endm
        ld h, d
        ld l, e

        .rept 3
            .SCALE b,c
            ld a, #16
            .ADD_A_HL
        .endm
        .SCALE b,c

        ret
    __endasm;
#else
    __asm
        ld hl, #0
        ret
    __endasm;
#endif
}

void protected_generate_thumbnail(uint8_t slot) BANKED {
    SWITCH_RAM((slot >> 1) + 1);
    uint16_t * sour = (slot & 1) ? image_second : image_first;
    uint16_t * dest = (slot & 1) ? image_second_thumbnail : image_first_thumbnail;
    for (uint8_t y = 0; y != 28; y++) {
        uint16_t * s = sour + ((uint16_t)((uint8_t)(y * 4) / 8)) * (CAMERA_IMAGE_TILE_WIDTH * 8) + ((y * 4) % 8);
        uint16_t * d = dest + ((uint16_t)(y / 8)) * (CAMERA_THUMB_TILE_WIDTH * 8) + (y % 8);
        for (uint8_t x = 0; x != 4; x++) {
            *d = protected_scale_line_part(s);
            d += 8, s += (4 * 8);
        }
    }
    for (uint8_t y = 28; y != 32; y++) {
        uint16_t * d = dest + ((uint16_t)(y / 8)) * (CAMERA_THUMB_TILE_WIDTH * 8) + (y % 8);
        for (uint8_t x = 0; x != 4; x++) {
            *d = 0, d += 8;
        }
    }
}

void protected_lastseen_to_slot(uint8_t slot) BANKED {
    static uint8_t slot_bank, * dest, * sour;
    uint8_t buffer[CAMERA_IMAGE_TILE_WIDTH * 16];

    slot_bank = (slot >> 1) + 1;
    dest = (slot & 1) ? image_second : image_first;
    sour = last_seen;

    for (uint8_t i = CAMERA_IMAGE_TILE_HEIGHT; i != 0; i--) {
        SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
        memcpy(buffer, sour, sizeof(buffer)), sour += sizeof(buffer);
        SWITCH_RAM(slot_bank);
        memcpy(dest, buffer, sizeof(buffer)), dest += sizeof(buffer);
    }
}