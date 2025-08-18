#ifndef __PROTECTED_H_INCLUDE__
#define __PROTECTED_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "compat.h"
#include "gbcamera.h"
#include "flip.h"

#define PROTECTED_CORRECT       0x00
#define PROTECTED_REPAIR_ALBUM  0x01
#define PROTECTED_REPAIR_VECTOR 0x02
#define PROTECTED_REPAIR_OWNER  0x04
#define PROTECTED_REPAIR_META   0x08
#define PROTECTED_REPAIR_CAL    0x10

extern uint8_t protected_status;

#define PROTECTED_SEED 0xAA55u

inline void protected_modify_slot(uint8_t slot, uint8_t value) {
    CAMERA_SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    uint8_t old = cam_image_slots.imageslots[slot];
    cam_image_slots_echo.imageslots[slot] = cam_image_slots.imageslots[slot] = value;
    cam_image_slots_echo.magic.crc_add = cam_image_slots.magic.crc_add += (value - old);
    cam_image_slots_echo.magic.crc_xor = cam_image_slots.magic.crc_xor ^= (value ^ old);
}

inline uint16_t protected_calculate_crc(uint8_t * data, uint8_t size, uint16_t seed) {
    uint8_t a = (uint8_t)seed, b = (uint8_t)(seed >> 8);
    while (size--) a += *data, b ^= *data++;
    return ((uint16_t)b << 8) | a;
}

void protected_pack(uint8_t * v) BANKED;
void protected_generate_thumbnail(uint8_t slot) BANKED;
void protected_lastseen_to_slot(uint8_t slot, camera_flip_e flip) BANKED;
uint8_t protected_metadata_read(uint8_t slot, uint8_t * dest, uint8_t size) BANKED;
uint8_t protected_metadata_write(uint8_t slot, uint8_t * dest, uint8_t size) BANKED;
void protected_owner_info_write(void) BANKED;
void protected_image_owner_write(uint8_t slot) BANKED;

#endif