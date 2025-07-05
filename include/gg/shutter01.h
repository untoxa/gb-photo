#ifndef __shutter01_INCLUDE__
#define __shutter01_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_shutter01 0b00000000

BANKREF_EXTERN(shutter01)
extern const uint8_t shutter01[];
extern void __mute_mask_shutter01;

#endif
