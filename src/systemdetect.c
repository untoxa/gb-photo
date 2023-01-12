#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "systemdetect.h"

BANKREF(module_detect_system)

uint8_t _is_SUPER, _is_COLOR, _is_ADVANCE;
uint8_t _is_CPU_FAST;

uint8_t INIT_module_detect_system() BANKED {
#if defined(NINTENDO)
    // For the SGB + PAL SNES setup this delay is required on startup, otherwise borders don't show up
    for (uint8_t i = 0; i != 4; i++) vsync();

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
    return 0;
}

uint8_t CPU_FAST() BANKED {
    return (_is_CPU_FAST = (_is_COLOR) ? (cpu_fast(), TRUE) : FALSE);
}

void CPU_SLOW() BANKED {
    if (_is_COLOR) _is_CPU_FAST = (cpu_slow(), FALSE);
}
