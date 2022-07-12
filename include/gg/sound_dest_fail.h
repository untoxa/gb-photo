#ifndef __sound_dest_fail_INCLUDE__
#define __sound_dest_fail_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_sound_dest_fail 0b00001010
BANKREF_EXTERN(sound_dest_fail)
extern const uint8_t sound_dest_fail[];
extern void __mute_mask_sound_dest_fail;

#endif
