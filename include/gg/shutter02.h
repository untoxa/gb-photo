#ifndef __shutter02_INCLUDE__
#define __shutter02_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_shutter02 0b00000000

BANKREF_EXTERN(shutter02)
extern const uint8_t shutter02[];
extern void __mute_mask_shutter02;

#endif
