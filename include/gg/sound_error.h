#ifndef __sound_error_INCLUDE__
#define __sound_error_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_sound_error 0b00000000
BANKREF_EXTERN(sound_error)
extern const uint8_t sound_error[];
extern void __mute_mask_sound_error;

#endif
