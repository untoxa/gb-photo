#ifndef __SGB_ASSETS_INCLUDE__
#define __SGB_ASSETS_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define SGB_BORDER_DEFAULT  0
#define SGB_BORDER_FANCY    1

#define SGB_PALETTE_GRAY    0
#define SGB_PALETTE_RED     1

BANKREF_EXTERN(module_sgb_assets)

uint8_t INIT_module_sgb_assets(void) BANKED;

#endif