#pragma bank 255

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdio.h>

#include "compat.h"
#include "musicmanager.h"
#include "screen.h"
#include "states.h"
#include "globals.h"
#include "systemhelpers.h"
#include "joy.h"

#include "misc_assets.h"

#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_settings.h"

// graphics assets
#include "logo_about.h"
#include "main_menu.h"
#include "cursors.h"
#include "hand_cursor.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

static uint8_t cursor_anim = 0;
static const menu_item_t * last_menu_item =  NULL;

#define HAND_CURSOR_BASE_TILE (0x80 - cursors_TILE_COUNT - hand_cursor_TILE_COUNT)
static const metasprite_t hand_cursor0[] = {
    METASPR_ITEM(32, 16, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t hand_cursor1[] = {
    METASPR_ITEM(33, 16, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t hand_cursor2[] = {
    METASPR_ITEM(33, 17, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t hand_cursor3[] = {
    METASPR_ITEM(32, 17, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t* const hand_cursor[] = {hand_cursor0, hand_cursor1, hand_cursor2, hand_cursor3};

uint8_t onShowAbout(const struct menu_t * self, uint8_t * param);
uint8_t onAboutMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t AboutMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 13, .ofs_y = 14, .width = 4,
        .caption = ICON_A " Ok",
        .onPaint = NULL,
        .onGetProps = onAboutMenuItemProps,
        .result = ACTION_NONE
    }
};
const menu_t AboutMenu = {
    .x = 1, .y = 1, .width = 18, .height = 16,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_OK,
    .items = AboutMenuItems, .last_item = LAST_ITEM(AboutMenuItems),
    .onShow = onShowAbout, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};
uint8_t onAboutMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu; self;
    return ITEM_TEXT_CENTERED;
}
uint8_t onShowAbout(const menu_t * self, uint8_t * param) {
    param;
    menu_draw_frame(self);
    screen_load_image_banked(self->x + ((self->width - (logo_about_WIDTH / logo_about_TILE_W)) >> 1), self->y, logo_about_WIDTH / logo_about_TILE_W, logo_about_HEIGHT / logo_about_TILE_H, logo_about_tiles, BANK(logo_about));
    screen_restore_rect(self->x + ((self->width - (logo_about_WIDTH / logo_about_TILE_W)) >> 1), self->y, logo_about_WIDTH / logo_about_TILE_W, logo_about_HEIGHT / logo_about_TILE_H);
    vwf_activate_font(1);
    menu_text_out(self->x + 13, self->y     , 0, BLACK_ON_WHITE, ITEM_DEFAULT, "v." QUOTE(VERSION));
    menu_text_out(self->x +  1, self->y +  2, 0, BLACK_ON_WHITE, ITEM_DEFAULT, "Original idea:");
    menu_text_out(self->x +  2, self->y +  3, 0, DK_GR_ON_WHITE, ITEM_DEFAULT, "Andreas Hahn");
    menu_text_out(self->x +  1, self->y +  4, 0, BLACK_ON_WHITE, ITEM_DEFAULT, "Development:");
    menu_text_out(self->x +  2, self->y +  5, 0, DK_GR_ON_WHITE, ITEM_DEFAULT, "Toxa, Andreas Hahn");
    menu_text_out(self->x +  2, self->y +  6, 0, DK_GR_ON_WHITE, ITEM_DEFAULT, "Raphael Boichot, Hatch");
    menu_text_out(self->x +  1, self->y +  7, 0, BLACK_ON_WHITE, ITEM_DEFAULT, "Art and Sound:");
    menu_text_out(self->x +  2, self->y +  8, 0, DK_GR_ON_WHITE, ITEM_DEFAULT, "Tronimal, 2BitPit, bbbbbr");
    menu_text_out(self->x +  2, self->y +  9, 0, DK_GR_ON_WHITE, ITEM_DEFAULT, "rembrandx, NeoRame");
    menu_text_out(self->x +  1, self->y + 10, 0, BLACK_ON_WHITE, ITEM_DEFAULT, "Special thanks:");
    menu_text_out(self->x +  2, self->y + 11, 0, DK_GR_ON_WHITE, ITEM_DEFAULT, "bbbbbr, AlexiG, HDR,");
    menu_text_out(self->x +  2, self->y + 12, 0, DK_GR_ON_WHITE, ITEM_DEFAULT, "crizzlycruz, christianr");
    vwf_activate_font(0);
    return MENU_PROP_NO_FRAME;
}

uint8_t onTranslateKeyMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onIdleMainMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onShowMain(const menu_t * self, uint8_t * param);
uint8_t onHelpMainMenu(const struct menu_t * menu, const struct menu_item_t * selection);
const menu_item_t MainMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 2 + (28 * 0), .ofs_y = 16, .width = 0,
        .caption = NULL, //" Camera",
        .helpcontext = " Make your own pictures",
        .onPaint = NULL,
        .result = ACTION_CAMERA
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 2 + (28 * 1), .ofs_y = 16, .width = 0,
        .caption = NULL, //" Camera roll",
        .helpcontext = " View your camera roll",
        .onPaint = NULL,
        .result = ACTION_GALLERY
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 2 + (28 * 2), .ofs_y = 16, .width = 0,
        .caption = NULL, //" Flash storage",
        .helpcontext = " Save/Restore camera rolls",
        .onPaint = NULL,
        .result = ACTION_FLASHER
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 2 + (28 * 3), .ofs_y = 16, .width = 0,
        .caption = NULL, //" Settings",
        .helpcontext = " Camera system settings",
        .onPaint = NULL,
        .result = ACTION_SETTINGS
    }, {
        .sub = &AboutMenu, .sub_params = NULL,
        .ofs_x = 2 + (28 * 4), .ofs_y = 16, .width = 0,
        .caption = NULL, //" About",
        .helpcontext = " About \"Photo!\" v." QUOTE(VERSION),
        .onPaint = NULL,
        .result = ACTION_ABOUT
    }
};
const menu_t MainMenu = {
    .x = 1, .y = 3, .width = 18, .height = 8,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = MainMenuItems, .last_item = LAST_ITEM(MainMenuItems),
    .onShow = onShowMain, .onIdle = onIdleMainMenu, .onHelpContext = onHelpMainMenu,
    .onTranslateKey = onTranslateKeyMainMenu, .onTranslateSubResult = NULL
};

uint8_t onTranslateKeyMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    // swap J_UP/J_DOWN with J_LEFT/J_RIGHT buttons, because our menus are horizontal
    return joypad_swap_dpad(value);
}
uint8_t onIdleMainMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    last_menu_item = selection;
    if ((sys_time & 0x07) == 0) cursor_anim = ++cursor_anim & 0x03;
    hide_sprites_range(move_metasprite(hand_cursor[cursor_anim], HAND_CURSOR_BASE_TILE, 0, (menu->x << 3) + selection->ofs_x, (menu->y << 3) + selection->ofs_y), MAX_HARDWARE_SPRITES);
    sync_vblank();
    return 0;
}
uint8_t onShowMain(const menu_t * self, uint8_t * param) {
    param; self;
    menu_draw_frame(self);
    screen_load_image_banked(self->x + ((self->width - (logo_about_WIDTH / logo_about_TILE_W)) >> 1), self->y, logo_about_WIDTH / logo_about_TILE_W, logo_about_HEIGHT / logo_about_TILE_H, logo_about_tiles, BANK(logo_about));
    screen_restore_rect(self->x + ((self->width - (logo_about_WIDTH / logo_about_TILE_W)) >> 1), self->y, logo_about_WIDTH / logo_about_TILE_W, logo_about_HEIGHT / logo_about_TILE_H);
    screen_load_image_banked(self->x, self->y + 2, main_menu_WIDTH / main_menu_TILE_W, main_menu_HEIGHT / main_menu_TILE_H, main_menu_tiles, BANK(main_menu));
    screen_restore_rect(self->x, self->y + 2, main_menu_WIDTH / main_menu_TILE_W, main_menu_HEIGHT / main_menu_TILE_H);
    return (MENU_PROP_SHADOW | MENU_PROP_SELECTION);
}
uint8_t onHelpMainMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, ITEM_DEFAULT, selection->helpcontext);
    return 0;
}

// Main Menu execute
uint8_t menu_main_execute(void) BANKED {
    uint8_t menu_result;
    do {
        menu_result = menu_execute(&MainMenu, NULL, last_menu_item);
        hide_sprites_range(0, MAX_HARDWARE_SPRITES);
        switch (menu_result) {
            case ACTION_CAMERA:
                PLAY_SFX(sound_ok);
                CHANGE_STATE(state_camera);
                return STATE_CHANGED();
            case ACTION_GALLERY:
                PLAY_SFX(sound_ok);
                CHANGE_STATE(state_gallery);
                return STATE_CHANGED();
            case ACTION_FLASHER:
                PLAY_SFX(sound_ok);
                CHANGE_STATE(state_flasher);
                return STATE_CHANGED();
            case ACTION_SETTINGS:
                menu_settings_execute();
                break;
            case ACTION_ABOUT:
                PLAY_SFX(sound_ok);
                return FALSE;
            default:
                // unknown command or cancel
                PLAY_SFX(sound_ok);
                break;
        }
    } while (menu_result != ACTION_NONE);
    return FALSE;
}
