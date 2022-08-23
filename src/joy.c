#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "joy.h"

BANKREF(module_joy)

uint8_t joy = 0, old_joy = 0;
uint16_t joy_ts = 0;

#if (INT_DRIVEN_JOYPAD==1)
volatile uint8_t joy_isr_value = 0;
void joypad_ISR() NONBANKED {
    // OR keypad bits until read
    if ((sys_time - joy_ts) > COOLDOWN_RATE) {
        joy_isr_value |= (joypad() | remote_joypad());
    }
}
#endif

uint8_t INIT_module_joy() BANKED {
    JOYPAD_INIT;
    return 0;
}