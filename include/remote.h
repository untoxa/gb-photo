#ifndef __REMOTE_H_INCLUDE__
#define __REMOTE_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define REMOTE_ENABLED  TRUE
#define REMOTE_DISABLED FALSE

extern volatile uint8_t remote_keys;

inline uint8_t remote_joypad() {
    return remote_keys;
}

#if defined(NINTENDO)

void remote_init() BANKED;
uint8_t remote_activate(uint8_t value) BANKED;

#else

inline void remote_init() {};

uint8_t remote_activate(uint8_t value) {
    return REMOTE_DISABLED;
};

#endif

#endif