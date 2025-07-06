#ifndef _BANKDATA_H_INCLUDE
#define _BANKDATA_H_INCLUDE

#include <stdint.h>

#include "states.h"

uint8_t call_far(const far_ptr_t *ptr);
uint8_t * banked_strcpy(uint8_t *dest, const uint8_t *src, uint8_t bank);
void * banked_memcpy(void *dest, const void *src, size_t len, uint8_t bank);
void banked_vmemcpy(void *dest, const void *src, size_t len, uint8_t bank);
uint8_t read_banked_ubyte(const uint8_t * ptr, uint8_t bank);
uint8_t * read_banked_ptr(const uint8_t ** ptr, uint8_t bank);
void set_banked_data(uint8_t *vram_addr, const uint8_t *data, uint16_t len, uint8_t bank);

#endif