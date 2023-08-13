#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "compat.h"
#include "systemdetect.h"

BANKREF(module_detect_system)

bool _is_SUPER, _is_COLOR, _is_ADVANCE;
bool _is_CPU_FAST;

uint8_t INIT_module_detect_system(void) BANKED {
#if defined(NINTENDO)
    // For the SGB + PAL SNES setup this delay is required on startup, otherwise borders don't show up
    for (uint8_t i = 0; i != 4; i++) sync_vblank();

    _is_SUPER    = sgb_check();
    _is_COLOR    = ((!_is_SUPER) && (_cpu == CGB_TYPE) && (*(uint8_t *)0x0143 & 0x80));
    _is_ADVANCE  = (_is_GBA && _is_COLOR);
    _is_CPU_FAST = false;
#elif defined(SEGA)
    _is_SUPER    = false;
    _is_COLOR    = true;
    _is_ADVANCE  = false;
    _is_CPU_FAST = false;
#endif
    return 0;
}

uint8_t CPU_FAST(void) BANKED {
    return (_is_CPU_FAST = (_is_COLOR) ? (cpu_fast(), true) : false);
}

void CPU_SLOW(void) BANKED {
    if (_is_COLOR) _is_CPU_FAST = (cpu_slow(), false);
}
