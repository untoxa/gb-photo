#ifndef _JOY_H_INCLUDE
#define _JOY_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#include "remote.h"

#ifndef INT_DRIVEN_JOYPAD
    #define INT_DRIVEN_JOYPAD 1
#endif

#define AUTOREPEAT_RATE 15
#define COOLDOWN_RATE 7

extern uint8_t joy_isr_value, joy, old_joy;
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
    return (((old_joy ^ joy) & (key)) && (joy & (key)));
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

#endif