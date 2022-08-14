#ifndef _JOY_H_INCLUDE
#define _JOY_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#include "globals.h"

#include "remote.h"

#define AUTOREPEAT_RATE 15

#if (INT_DRIVEN_JOYPAD==1)
#define COOLDOWN_RATE 12

extern volatile uint8_t joy_isr_value;
#endif

extern uint8_t joy, old_joy;
extern uint16_t joy_ts;

inline void JOYPAD_INPUT() {
    old_joy = ((sys_time - joy_ts) > AUTOREPEAT_RATE) ? 0 : joy;
#if (INT_DRIVEN_JOYPAD==1)
    joy = joy_isr_value, joy_isr_value = 0;
#else
    joy = (joypad() | remote_joypad());
#endif
}
inline void JOYPAD_AUTOREPEAT() {
    if (old_joy ^ joy) joy_ts = sys_time;
}

inline void PROCESS_INPUT() {
    JOYPAD_INPUT();
    JOYPAD_AUTOREPEAT();
}

inline uint8_t KEY_PRESSED(uint8_t key) {
    return (((old_joy ^ joy) & key) && (joy & key));
}

inline void JOYPAD_RESET() {
    joy = old_joy = 0;
#if (INT_DRIVEN_JOYPAD==1)
    joy_isr_value = 0;
#endif
}

#if (INT_DRIVEN_JOYPAD==1)

void joypad_ISR();

inline void joy_init() {
    CRITICAL{
        add_VBL(joypad_ISR);
    }
}
#define JOYPAD_INIT joy_init()

#else
#define JOYPAD_INIT
#endif

inline uint8_t joypad_swap_dpad(uint8_t value) {
    // swap J_UP/J_DOWN with J_LEFT/J_RIGHT buttons, because our menus are horizontal
    return (value & 0b11110000) | ((value << 1) & 0b00000100) | ((value >> 1) & 0b00000010) | ((value << 3) & 0b00001000) | ((value >> 3) & 0b00000001);
}

#endif