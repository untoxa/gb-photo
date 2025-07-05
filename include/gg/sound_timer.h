#ifndef __sound_timer_INCLUDE__
#define __sound_timer_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_sound_timer 0b00000000
BANKREF_EXTERN(sound_timer)
extern const uint8_t sound_timer[];
extern void __mute_mask_sound_timer;

#endif
