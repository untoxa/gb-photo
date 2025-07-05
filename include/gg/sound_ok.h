#ifndef __sound_ok_INCLUDE__
#define __sound_ok_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_sound_ok 0b00000000
BANKREF_EXTERN(sound_ok)
extern const uint8_t sound_ok[];
extern void __mute_mask_sound_ok;

#endif
