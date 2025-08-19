#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "compat.h"
#include "joy.h"
#include "vwf.h"
#include "screen.h"
#include "musicmanager.h"
#include "misc_assets.h"

#include "menus.h"

#include "sound_menu_move.h"

void menu_text_out(uint8_t x, uint8_t y, uint8_t w, uint8_t c, uint8_t props, const uint8_t * text) {
    uint8_t len, width = 0, lead = 0;
    if (text) {
        if (props & ITEM_TEXT_CENTERED) {
            width = vwf_text_width(text);
            if ((w << 3) > width) {
                width = ((w << 3) - width) >> 1;
                lead = width >> 3;
                if (lead) screen_clear_rect(x, y, lead, 1, c);
            }
        }
        vwf_set_colors(FG_COLOR(c), BG_COLOR(c));
        len = vwf_draw_text(screen_tile_addresses[y] + ((x + lead) << DEVICE_TILE_SIZE_BITS), text, width & 0x07);
        screen_restore_rect(x + lead, y, len, 1);
    } else len = 0;
    if ((lead + len) < w) screen_clear_rect(x + lead + len, y, w - lead - len, 1, c);
}

inline uint8_t menu_item_get_props(const menu_t * menu, const menu_item_t * item) {
    return (item->onGetProps) ? item->onGetProps(menu, item) : ITEM_DEFAULT;
}

const menu_item_t * menu_move_selection(const menu_t * menu, const menu_item_t * selection, const menu_item_t * new_selection) {
    static const uint8_t items_colors[2][2] = {{BLACK_ON_WHITE, WHITE_ON_BLACK}, {LT_GR_ON_WHITE, DK_GR_ON_BLACK}};
    if (selection) {
        uint8_t item_props = menu_item_get_props(menu, selection);
        menu_text_out(menu->x + selection->ofs_x, menu->y + selection->ofs_y, selection->width,
                      items_colors[(bool)(item_props & ITEM_DISABLED)][(bool)(menu->flags & MENU_FLAGS_INVERSE)],
                      item_props,
                      (selection->onPaint) ? selection->onPaint(menu, selection) : (uint8_t *)selection->caption);
    }
    if (new_selection)
        menu_text_out(menu->x + new_selection->ofs_x, menu->y + new_selection->ofs_y, new_selection->width,
                      ((menu->flags & MENU_FLAGS_INVERSE) ? BLACK_ON_WHITE : WHITE_ON_BLACK),
                      (new_selection->onGetProps) ? new_selection->onGetProps(menu, new_selection) : ITEM_DEFAULT,
                      (new_selection->onPaint) ? new_selection->onPaint(menu, new_selection) : (uint8_t *)new_selection->caption);
    return (new_selection) ? new_selection : selection;
}

const menu_item_t * menu_previous(const menu_t * menu, const menu_item_t * item) {
    const menu_item_t * itm = item;
    do {
        itm = ((itm - 1) < menu->items) ? (const menu_item_t *)(menu->last_item) : (itm - 1);
    } while (menu_item_get_props(menu, itm) & ITEM_DISABLED);
    return itm;
}

const menu_item_t * menu_next(const menu_t * menu, const menu_item_t * item) {
    const menu_item_t * itm = item;
    do {
        itm = ((itm + 1) > menu->last_item) ? (const menu_item_t *)(menu->items) : (itm + 1);
    } while (menu_item_get_props(menu, itm) & ITEM_DISABLED);
    return itm;
}

void menu_draw_frame(const menu_t * menu) {
    // field
    screen_clear_rect(menu->x, menu->y, menu->width, menu->height, BLACK_ON_WHITE);
    // corners
    screen_set_tile_xy(menu->x,                   menu->y,                    CORNER_UL);
    screen_set_tile_xy(menu->x + menu->width - 1, menu->y,                    CORNER_UR);
    screen_set_tile_xy(menu->x,                   menu->y + menu->height - 1, CORNER_DL);
    screen_set_tile_xy(menu->x + menu->width - 1, menu->y + menu->height - 1, CORNER_DR);
}
void menu_draw_shadow(const menu_t * menu) {
    // shadow
    screen_clear_rect(menu->x + 1,           menu->y + menu->height, menu->width,  1,                SLD_DARK_GRAY);
    screen_clear_rect(menu->x + menu->width, menu->y + 1,            1,            menu->height - 1, SLD_DARK_GRAY);
}

uint8_t menu_redraw(const menu_t * menu, uint8_t * param, const menu_item_t * selection) {
    uint8_t result;

    // call onShow handler if present
    result = (menu->onShow) ? menu->onShow(menu, param) : MENU_PROP_DEFAULT;
    // draw menu window
    if (menu->width) {
        if (result & MENU_PROP_FRAME) menu_draw_frame(menu);
        if (result & MENU_PROP_SHADOW) menu_draw_shadow(menu);
    }
    // draw menu items
    for (const menu_item_t * current_item = menu->items; current_item <= menu->last_item; current_item++) {
        if (current_item == selection) {
            if (result & MENU_PROP_SELECTION) {
                menu_move_selection(menu, NULL, current_item);
                if (menu->onHelpContext) menu->onHelpContext(menu, selection);
            }
        } else {
            if (result & MENU_PROP_ITEMS) menu_move_selection(menu, current_item, NULL);
        }
    }

    return result;
}

uint8_t menu_execute(const menu_t * menu, uint8_t * param, const menu_item_t * select) {
    const menu_item_t * selection, * new_selection;

    hide_sprites_range(0, MAX_HARDWARE_SPRITES);

    // initialize selection
    selection = (select) ? select : (menu->items);
    // move selection if the current item is disabled
    if (menu_item_get_props(menu, selection) & ITEM_DISABLED) selection = menu_next(menu, selection);

    menu_redraw(menu, param, selection);

    // process menu
    uint8_t result = MENU_RESULT_NONE;
    do {
        // process input
        JOYPAD_INPUT();
        if (menu->onTranslateKey) joy = menu->onTranslateKey(menu, selection, joy);
        JOYPAD_AUTOREPEAT();
        // process menu keys
        if (KEY_PRESSED(J_UP)) {
            if ((new_selection = menu_previous(menu, selection)) != selection) {
                PLAY_SFX(sound_menu_move);
                selection = menu_move_selection(menu, selection, new_selection);
                if (menu->onHelpContext) menu->onHelpContext(menu, selection);
            }
        } else if (KEY_PRESSED(J_DOWN)) {
            if ((new_selection = menu_next(menu, selection)) != selection) {
                PLAY_SFX(sound_menu_move);
                selection = menu_move_selection(menu, selection, new_selection);
                if (menu->onHelpContext) menu->onHelpContext(menu, selection);
            }
        } else if (KEY_PRESSED(J_A)) {
            if (selection->sub) {
                result = menu_execute(selection->sub, selection->sub_params, NULL);
                if (menu->onTranslateSubResult) result = menu->onTranslateSubResult(menu, selection, result);
            } else if (menu->onExecute) {
                result = menu->onExecute(menu, selection);
            } else result = selection->result;
            if (result == MENU_RESULT_RETURN) {
                menu_redraw(menu, param, selection);
                JOYPAD_RESET();
                result =  MENU_RESULT_NONE;
            }
        } else if (KEY_PRESSED(menu->cancel_mask)) {
            result = menu->cancel_result;
        }
        if (result != MENU_RESULT_NONE) break;
        if (menu->onIdle) result = menu->onIdle(menu, selection); else sync_vblank();
    } while (result == MENU_RESULT_NONE);

    return result;
}
