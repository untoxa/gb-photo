#ifndef __sound_select_fail_INCLUDE__
#define __sound_select_fail_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_sound_select_fail 0b00000010
BANKREF_EXTERN(sound_select_fail)
extern const uint8_t sound_select_fail[];
extern void __mute_mask_sound_select_fail;

#endif
