#ifndef __PROTECTED_H_INCLUDE__
#define __PROTECTED_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "gbcamera.h"

#define PROTECTED_SEED 0xAA55u

inline void protected_modify_slot(uint8_t slot, uint8_t value) {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    uint8_t old = cam_game_data.imageslots[slot];
    cam_game_data_echo.imageslots[slot] = cam_game_data.imageslots[slot] = value;
    cam_game_data_echo.CRC_add = cam_game_data.CRC_add += (value - old);
    cam_game_data_echo.CRC_xor = cam_game_data.CRC_xor ^= (value ^ old);
}

inline uint16_t protected_calculate_crc(uint8_t * data, uint8_t size, uint16_t seed) {
    uint8_t a = (uint8_t)seed, b = (uint8_t)(seed >> 8);
    while (size--) a += *data, b ^= *data++;
    return ((uint16_t)b << 8) | a;
}

void protected_pack(uint8_t * v) BANKED;
void protected_generate_thumbnail(uint8_t slot) BANKED;
void protected_lastseen_to_slot(uint8_t slot, bool flipped) BANKED;
uint8_t protected_metadata_read(uint8_t slot, uint8_t * dest, uint8_t size) BANKED;
uint8_t protected_metadata_write(uint8_t slot, uint8_t * dest, uint8_t size) BANKED;

#endif