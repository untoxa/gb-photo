#ifndef _JOY_H_INCLUDE
#define _JOY_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#include "remote.h"

#define AUTOREPEAT_RATE 15

extern uint8_t joy, old_joy;
extern uint16_t joy_ts;

inline void JOYPAD_INPUT() {
    old_joy = ((sys_time - joy_ts) > AUTOREPEAT_RATE) ? 0 : joy;
    joy = (joypad() | remote_joypad());
}
inline void JOYPAD_AUTOREPEAT() {
    if (old_joy ^ joy) joy_ts = sys_time; 
}

inline void PROCESS_INPUT() {
    JOYPAD_INPUT();
    JOYPAD_AUTOREPEAT(); 
}

inline uint8_t KEY_PRESSED(uint8_t key) {
    return (((old_joy ^ joy) & (key)) && (joy & (key)));
}

#endif