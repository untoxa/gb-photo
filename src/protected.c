#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "gbcamera.h"
#include "vector.h"

#include "protected.h"

void protected_pack(uint8_t * v) BANKED {
    uint8_t i, elem, idx = 0;
    VECTOR_ITERATE(v, i, elem) {
        protected_modify_slot(elem, idx++);
    }
}

void protected_lastseen_to_slot(uint8_t slot) BANKED {
    static uint8_t slot_bank, * dest, * sour;
    uint8_t buffer[CAMERA_IMAGE_TILE_WIDTH * 16];

    slot_bank = (slot >> 1) + 1;
    dest = (slot & 1) ? image_first : image_second;
    sour = last_seen;

    for (uint8_t i = CAMERA_IMAGE_TILE_HEIGHT; i != 0; i--) {
        SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
        memcpy(buffer, sour, sizeof(buffer)), sour += sizeof(buffer);
        SWITCH_RAM(slot_bank);
        memcpy(dest, buffer, sizeof(buffer)), dest += sizeof(buffer);
    }
}