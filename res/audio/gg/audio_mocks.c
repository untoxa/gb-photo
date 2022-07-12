#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

BANKREF(sound_ok)
const uint8_t sound_ok[] = { 0 };
const void AT(0b00001010) __mute_mask_sound_ok;

BANKREF(sound_error)
const uint8_t sound_error[] = { 0 };
const void AT(0b00000010) __mute_mask_sound_error;

BANKREF(music_ingame)
const uint8_t music_ingame[] = { 0 }; 
