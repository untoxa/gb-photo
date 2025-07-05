#ifndef __sound_transmit_INCLUDE__
#define __sound_transmit_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define MUTE_MASK_sound_transmit 0b00000000
BANKREF_EXTERN(sound_transmit)
extern const uint8_t sound_transmit[];
extern void __mute_mask_sound_transmit;

#endif
