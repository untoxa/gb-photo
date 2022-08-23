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
    for (uint8_t i = 0; i != 4; i++) wait_vbl_done();

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

uint8_t CPU_FAST() NONBANKED {
#if (USE_CGB_DOUBLE_SPEED==1)
    _is_CPU_FAST = (_is_COLOR) ? (cpu_fast(), TRUE) : FALSE;
    return _is_CPU_FAST;
#else
    return (_is_CPU_FAST = FALSE);
#endif
}

uint8_t CPU_SLOW() NONBANKED {
#if (USE_CGB_DOUBLE_SPEED==1)
    if (_is_COLOR) cpu_slow();
#endif
    return (_is_CPU_FAST = FALSE);
}
