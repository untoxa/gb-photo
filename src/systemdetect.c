#include <gbdk/platform.h>
#include <stdint.h>

#include "systemdetect.h"

uint8_t _is_SUPER, _is_COLOR, _is_ADVANCE;
uint8_t _is_CPU_FAST;

void detect_system() {
#if defined(NINTENDO)
    _is_SUPER    = sgb_check();
    _is_COLOR    = ((!_is_SUPER) && (_cpu == CGB_TYPE) && (*(uint8_t *)0x0143 & 0x80));
    _is_ADVANCE  = (_is_GBA && _is_COLOR);
    _is_CPU_FAST = FALSE;
#elif defined(SEGA)
    _is_SUPER    = FALSE;
    _is_COLOR    = TRUE;
    _is_ADVANCE  = FALSE;
    _is_CPU_FAST = FALSE;
#endif
}

void CPU_FAST(uint8_t display) {
#if (USE_CGB_DOUBLE_SPEED==1)
    if (_is_COLOR) {
        if (display) DISPLAY_OFF;
        cpu_fast();
        TMA_REG = 0xC0u;
        _is_CPU_FAST = TRUE;
        if (display) DISPLAY_ON;
    } else _is_CPU_FAST = FALSE;
#else
    display;
    _is_CPU_FAST = FALSE;
#endif
}

void CPU_SLOW(uint8_t display) {
    display;
#if (USE_CGB_DOUBLE_SPEED==1)
    if (_is_COLOR) {
        if (display) DISPLAY_OFF;
        cpu_slow();
        TMA_REG = 0x80u;
        if (display) DISPLAY_ON;
    }
#endif
    _is_CPU_FAST = FALSE;
}
