#ifndef __PROTECTED_H_INCLUDE__
#define __PROTECTED_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#include "gbcamera.h"

inline void protected_modify_slot(uint8_t slot, uint8_t value) {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    uint8_t old = cam_game_data.imageslots[slot];
    cam_game_data_echo.imageslots[slot] = cam_game_data.imageslots[slot] = value;
    cam_game_data_echo.CRC_add = cam_game_data.CRC_add += (value - old);
    cam_game_data_echo.CRC_xor = cam_game_data.CRC_xor ^= (value ^ old);
}

void protected_pack(uint8_t * v) BANKED;
void protected_lastseen_to_slot(uint8_t slot) BANKED;

#endif