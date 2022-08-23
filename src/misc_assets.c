#pragma bank 255

#include <gbdk/platform.h>

#include "screen.h"
#include "bankdata.h"

#include "common_tiles.h"
#include "cursors.h"

BANKREF(module_misc_assets)

uint8_t INIT_module_misc_assets() BANKED {
#ifdef NINTENDO
    set_banked_data(TO_TILE_ADDRESS(_VRAM8000, (256 - common_tiles_TILE_COUNT)), common_tiles_tiles, (common_tiles_TILE_COUNT << 4), BANK(common_tiles));
    set_banked_data(TO_TILE_ADDRESS(_VRAM8000, (128 - cursors_TILE_COUNT)), cursors_tiles, (cursors_TILE_COUNT << 4), BANK(cursors));
#endif
    return 0;
}
