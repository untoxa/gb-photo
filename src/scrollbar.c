#pragma bank 255

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include "scrollbar.h"

#include "screen.h"
#include "cursors.h"
#include "scrollers.h"

scrollbar_t * scrollbars = NULL;

uint8_t scrollbar_render_all(uint8_t hiwater) NONBANKED {
    uint8_t save = _current_bank, hw = hiwater;
    SWITCH_ROM(BANK(cursors));
    scrollbar_t * current = scrollbars;
    while (current) {
        hw += move_metasprite(cursors_metasprites[1], (0x80 - cursors_TILE_COUNT), hw,
                              ((current->vertical) ? (current->x << 3) : ((current->x << 3) + current->position + 8)),
                              ((current->vertical) ? ((current->y << 3) + current->position + 8) : (current->y << 3)));
        current = current->prev;
    }
    SWITCH_ROM(save);
    return hw;
}

void scrollbar_paint(scrollbar_t * ss) {
    if (!ss) return;
    if (ss->vertical) {
        screen_load_tile_banked(ss->x, ss->y, scrollers_tiles + (3 * 16), BANK(scrollers));
        for (int8_t i = ss->y + 1; i < (ss->y + ss->len - 1); i++) {
            screen_load_tile_banked(ss->x, i, scrollers_tiles + (4 * 16), BANK(scrollers));
        }
        screen_load_tile_banked(ss->x, ss->y + ss->len - 1, scrollers_tiles + (5 * 16), BANK(scrollers));
        screen_restore_rect(ss->x, ss->y, 1, ss->len);
    } else {
        screen_load_tile_banked(ss->x, ss->y, scrollers_tiles + (0 * 16), BANK(scrollers));
        for (int8_t i = ss->x + 1; i < (ss->x + ss->len - 1); i++) {
            screen_load_tile_banked(i, ss->y, scrollers_tiles + (1 * 16), BANK(scrollers));
        }
        screen_load_tile_banked(ss->x + ss->len - 1, ss->y, scrollers_tiles + (2 * 16), BANK(scrollers));
        screen_restore_rect(ss->x, ss->y, ss->len, 1);
    }
}

void scrollbar_repaint_all() BANKED {
    scrollbar_t * current = scrollbars;
    while (current) {
        scrollbar_paint(current);
        current = current->prev;
    }
}

void scrollbar_set_position(scrollbar_t * ss, uint16_t value, uint16_t min, uint16_t max) BANKED {
    ss->position = (((uint32_t)(ss->len - 3) << 3) * (value - min)) / (max - min);
}