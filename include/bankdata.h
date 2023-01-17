#ifndef _BANKDATA_H_INCLUDE
#define _BANKDATA_H_INCLUDE

#include <stdint.h>

#include "states.h"

uint8_t call_far(const far_ptr_t *ptr);
uint8_t * banked_strcpy(uint8_t *dest, const uint8_t *src, uint8_t bank);
void * banked_memcpy(void *dest, const void *src, size_t len, uint8_t bank);
void banked_vmemcpy(void *dest, const void *src, size_t len, uint8_t bank);
void set_banked_sprite_data(uint8_t first_tile, uint8_t nb_tiles, const uint8_t *data, uint8_t bank);
void set_banked_bkg_data(uint8_t first_tile, uint8_t nb_tiles, const uint8_t *data, uint8_t bank);
void set_banked_bkg_submap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *map, uint8_t map_w, uint8_t bank);
uint8_t read_banked_ubyte(const uint8_t * ptr, uint8_t bank);
void set_banked_data(uint8_t *vram_addr, const uint8_t *data, uint16_t len, uint8_t bank);

#endif