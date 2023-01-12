#ifndef _SPLASH_H_INCLUDE
#define _SPLASH_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

BANKREF_EXTERN(module_splash)

typedef uint8_t mask_t[8][8];

uint8_t INIT_module_splash() BANKED;

#endif