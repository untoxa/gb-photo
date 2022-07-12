#include <gbdk/platform.h>
#include <stdint.h>

#include "systemdetect.h"

uint8_t _is_SUPER, _is_COLOR, _is_ADVANCE;

void detect_system() {
#if defined(NINTENDO)
    _is_SUPER   = sgb_check();
    _is_COLOR   = ((!_is_SUPER) && (_cpu == CGB_TYPE) && (*(uint8_t *)0x0143 & 0x80));
    _is_ADVANCE = (_is_GBA && _is_COLOR);
#elif defined(SEGA)
    _is_SUPER   = FALSE;
    _is_COLOR   = TRUE;
    _is_ADVANCE = FALSE;
#endif
}