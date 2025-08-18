#ifndef _USERINFO_H_INCLUDE
#define _USERINFO_H_INCLUDE

#include <gbdk/platform.h>

#include <stdint.h>

typedef enum {
    gender_type_not_set,
    gender_type_male,
    gender_type_female,
    N_GENDER_TYPES
} gender_type_e;

uint8_t * userinfo_get_userid(uint8_t * buf) BANKED;
uint8_t * userinfo_get_username(uint8_t * buf) BANKED;
uint8_t * userinfo_get_birthdate(uint8_t * buf) BANKED;
uint8_t userinfo_get_gender(void) BANKED;

void userinfo_set_username(uint8_t * buf) BANKED;
void userinfo_set_gender(uint8_t gender) BANKED;

#endif