#ifndef __REMOTE_H_INCLUDE__
#define __REMOTE_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define REMOTE_ENABLED  TRUE
#define REMOTE_DISABLED FALSE

BANKREF_EXTERN(module_remote)

extern volatile uint8_t remote_keys;

inline uint8_t remote_joypad(void) {
    return remote_keys;
}

#if defined(NINTENDO)

uint8_t remote_activate(uint8_t value) BANKED;
uint8_t INIT_module_remote(void) BANKED;

#else

uint8_t remote_activate(uint8_t value) {
    return REMOTE_DISABLED;
};
uint8_t INIT_module_remote(void) BANKED;

#endif

#endif