#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdio.h>

#include "joy.h"
#include "vwf.h"
#include "screen.h"
#include "musicmanager.h"
#include "misc_assets.h"

#include "menus.h"

#include "sound_menu_move.h"

void menu_text_out(uint8_t x, uint8_t y, uint8_t w, uint8_t c, const uint8_t * text) {
    uint8_t len;
    if (text) {
        vwf_set_mask((c == SOLID_BLACK) ? 0xff : 0);
        len = vwf_draw_text(screen_tile_addresses[y] + (x << 4), text);
        screen_restore_rect(x, y, len, 1);
    } else len = 0;
    if (len < w) screen_clear_rect(x + len, y, w - len, 1, c);
}

const menu_item_t * menu_move_selection(const menu_t * menu, const menu_item_t * selection, const menu_item_t * new_selection) {
    if (selection)
        menu_text_out(menu->x + selection->ofs_x, menu->y + selection->ofs_y, selection->width,
                      ((menu->flags & MENU_INVERSE) ? SOLID_BLACK : SOLID_WHITE),
                      (selection->onPaint) ? selection->onPaint(menu, selection) : (uint8_t *)selection->caption);
    if (new_selection)
        menu_text_out(menu->x + new_selection->ofs_x, menu->y + new_selection->ofs_y, new_selection->width,
                      ((menu->flags & MENU_INVERSE) ? SOLID_WHITE : SOLID_BLACK),
                      (new_selection->onPaint) ? new_selection->onPaint(menu, new_selection) : (uint8_t *)new_selection->caption);
    return (new_selection) ? new_selection : selection;
}

uint8_t menu_execute(const menu_t * menu, uint8_t * param, const menu_item_t * select) {
    const menu_item_t * selection;
    uint8_t result = 0;

    hide_sprites_range(0, MAX_HARDWARE_SPRITES);

    selection = (select) ? select : (menu->items);

    if (menu->width) {
        // zero menu frame width == not draw menu frame
        screen_clear_rect(menu->x, menu->y, menu->width, menu->height, SOLID_WHITE);
        set_bkg_tile_xy(menu->x,                   menu->y,                    CORNER_UL);
        set_bkg_tile_xy(menu->x + menu->width - 1, menu->y,                    CORNER_UR);
        set_bkg_tile_xy(menu->x,                   menu->y + menu->height - 1, CORNER_DL);
        set_bkg_tile_xy(menu->x + menu->width - 1, menu->y + menu->height - 1, CORNER_DR);
    }

    // call onShow handler if present
    if (menu->onShow) menu->onShow(menu, param);

    // draw menu items
    for (const menu_item_t * current_item = menu->items; current_item <= menu->last_item; current_item++) {
        if (current_item == selection) {
            menu_move_selection(menu, NULL, current_item);
            if (menu->onHelpContext) menu->onHelpContext(menu, selection);
        } else {
            menu_move_selection(menu, current_item, NULL);
        }
    }

    do {
        // process input
        JOYPAD_INPUT();
        if (menu->onTranslateKey) joy = menu->onTranslateKey(menu, selection, joy);
        JOYPAD_AUTOREPEAT();
        // process menu keys
        if (KEY_PRESSED(J_UP)) {
            if (menu->items != menu->last_item) {
                music_play_sfx(BANK(sound_menu_move), sound_menu_move, SFX_MUTE_MASK(sound_menu_move), MUSIC_SFX_PRIORITY_MINIMAL);
                selection = menu_move_selection(menu, selection, ((selection - 1) < menu->items) ? menu->last_item : (selection - 1));
                if (menu->onHelpContext) menu->onHelpContext(menu, selection);
            }
        } else if (KEY_PRESSED(J_DOWN)) {
            if (menu->items != menu->last_item) {
                music_play_sfx(BANK(sound_menu_move), sound_menu_move, SFX_MUTE_MASK(sound_menu_move), MUSIC_SFX_PRIORITY_MINIMAL);
                selection = menu_move_selection(menu, selection, ((selection + 1) > menu->last_item) ? menu->items : (selection + 1));
                if (menu->onHelpContext) menu->onHelpContext(menu, selection);
            }
        } else if (KEY_PRESSED(J_A)) {
            if (selection->sub) {
                result = menu_execute(selection->sub, selection->sub_params, NULL);
                if (menu->onTranslateSubResult) result = menu->onTranslateSubResult(menu, selection, result);
            } else result = selection->result;
        } else if (KEY_PRESSED(menu->cancel_mask)) {
            return menu->cancel_result;
        }
        if (menu->onIdle) {
            uint8_t res;
            if (res = menu->onIdle(menu, selection)) return res;
        } else wait_vbl_done();
    } while (result == 0);

    return result;
}
