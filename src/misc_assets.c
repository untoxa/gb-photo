#pragma bank 255

#include <gbdk/platform.h>

#include "screen.h"
#include "bankdata.h"

#include "common_tiles.h"
#include "cursors.h"
#include "hand_cursor.h"

BANKREF(module_misc_assets)

uint8_t INIT_module_misc_assets() BANKED {
#ifdef NINTENDO
    set_banked_data(TO_TILE_ADDRESS(_VRAM8000, (0x100 - common_tiles_TILE_COUNT)), common_tiles_tiles, (common_tiles_TILE_COUNT << 4), BANK(common_tiles));
    set_banked_data(TO_TILE_ADDRESS(_VRAM8000, (0x80 - cursors_TILE_COUNT)), cursors_tiles, (cursors_TILE_COUNT << 4), BANK(cursors));
    set_banked_data(TO_TILE_ADDRESS(_VRAM8000, (0x80 - cursors_TILE_COUNT - hand_cursor_TILE_COUNT)), hand_cursor_tiles, (hand_cursor_TILE_COUNT << 4), BANK(hand_cursor));
#endif
    return 0;
}
