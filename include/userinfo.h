#ifndef _USERINFO_H_INCLUDE
#define _USERINFO_H_INCLUDE

#include <gbdk/platform.h>

#include <stdint.h>

uint8_t * userinfo_get_userid(uint8_t * buf) BANKED;
uint8_t * userinfo_get_username(uint8_t * buf) BANKED;
uint8_t * userinfo_get_birthdate(uint8_t * buf) BANKED;
uint8_t userinfo_get_gender(void) BANKED;

#endif