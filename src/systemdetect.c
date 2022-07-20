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

uint8_t CPU_FAST() {
#if (USE_CGB_DOUBLE_SPEED==1)
    _is_CPU_FAST = (_is_COLOR) ? (cpu_fast(), TRUE) : FALSE;
    return _is_CPU_FAST;
#else
    return (_is_CPU_FAST = FALSE);
#endif
}

uint8_t CPU_SLOW() {
#if (USE_CGB_DOUBLE_SPEED==1)
    if (_is_COLOR) cpu_slow();
#endif
    return (_is_CPU_FAST = FALSE);
}
