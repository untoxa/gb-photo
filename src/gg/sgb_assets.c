#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"

BANKREF(module_sgb_assets)

// load the SGB borders and palettes if SGB detected
uint8_t INIT_module_sgb_assets(void) BANKED {
    return 0;
}