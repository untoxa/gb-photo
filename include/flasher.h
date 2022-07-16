#ifndef __FLASHER_H_INCLUDE
#define __FLASHER_H_INCLUDE

#include <stdint.h>

uint8_t flash_check_gallery_exist(uint8_t slot) NONBANKED;
uint8_t flash_load_gallery_from_slot(uint8_t slot) BANKED;
uint8_t flash_save_gallery_to_slot(uint8_t slot) BANKED;
uint8_t flash_erase_slot(uint8_t slot) BANKED;

#endif