#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "musicmanager.h"
#include "screen.h"
#include "states.h"
#include "globals.h"
#include "systemhelpers.h"

#include "misc_assets.h"

#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_settings.h"

// graphics assets
#include "logo_about.h"
#include "main_menu.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

uint8_t onShowAbout(const struct menu_t * self, uint8_t * param);
const menu_item_t AboutMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 13, .ofs_y = 16, .width = 4,
        .caption = " " ICON_A " Ok",
        .onPaint = NULL,
        .result = MENU_RESULT_OK
    }
};
const menu_t AboutMenu = {
    .x = 1, .y = 0, .width = 18, .height = 18,
    .items = AboutMenuItems, .last_item = LAST_ITEM(AboutMenuItems),
    .onShow = onShowAbout, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};
uint8_t onShowAbout(const menu_t * self, uint8_t * param) {
    param;
    menu_draw_frame(self);
    screen_load_image_banked(self->x + ((self->width - (logo_about_WIDTH / logo_about_TILE_W)) >> 1), self->y, logo_about_WIDTH / logo_about_TILE_W, logo_about_HEIGHT / logo_about_TILE_H, logo_about_tiles, BANK(logo_about), IMAGE_NORMAL);
    screen_restore_rect(self->x + + ((self->width - (logo_about_WIDTH / logo_about_TILE_W)) >> 1), self->y, logo_about_WIDTH / logo_about_TILE_W, logo_about_HEIGHT / logo_about_TILE_H);
    vwf_activate_font(1);
    menu_text_out(self->x + 1, self->y +  2, 0, BLACK_ON_WHITE, "Original idea:");
    menu_text_out(self->x + 2, self->y +  3, 0, DK_GR_ON_WHITE, "Andreas Hahn");
    menu_text_out(self->x + 1, self->y +  4, 0, BLACK_ON_WHITE, "Development:");
    menu_text_out(self->x + 2, self->y +  5, 0, DK_GR_ON_WHITE, "Toxa, Andreas Hahn");
    menu_text_out(self->x + 2, self->y +  6, 0, DK_GR_ON_WHITE, "Raphael Boichot, Hatch");
    menu_text_out(self->x + 1, self->y +  7, 0, BLACK_ON_WHITE, "Sound:");
    menu_text_out(self->x + 2, self->y +  8, 0, DK_GR_ON_WHITE, "Tronimal");
    menu_text_out(self->x + 1, self->y +  9, 0, BLACK_ON_WHITE, "Art:");
    menu_text_out(self->x + 2, self->y + 10, 0, DK_GR_ON_WHITE, "rembrandx, 2BitPit");
    menu_text_out(self->x + 2, self->y + 11, 0, DK_GR_ON_WHITE, "NeoRame");
    menu_text_out(self->x + 1, self->y + 12, 0, BLACK_ON_WHITE, "Special thanks:");
    menu_text_out(self->x + 2, self->y + 13, 0, DK_GR_ON_WHITE, "bbbbbr, AlexiG, HDR,");
    menu_text_out(self->x + 2, self->y + 14, 0, DK_GR_ON_WHITE, "crizzlycruz, christianr");
    vwf_activate_font(0);
    return MENU_PROP_NO_FRAME;
}

uint8_t onHelpMainMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onTranslateSubResultMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t MainMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 10,
        .caption = " Camera",
        .helpcontext = " Make your own pictures",
        .onPaint = NULL,
        .result = ACTION_CAMERA
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 10,
        .caption = " Image gallery",
        .helpcontext = " View your image gallery",
        .onPaint = NULL,
        .result = ACTION_GALLERY
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 10,
        .caption = " Flash storage",
        .helpcontext = " Save/Restore galleries",
        .onPaint = NULL,
        .result = ACTION_FLASHER
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 10,
        .caption = " Settings",
        .helpcontext = " Edit program settings",
        .onPaint = NULL,
        .result = ACTION_SETTINGS
    }, {
        .sub = &AboutMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 5, .width = 10,
        .caption = " About",
        .helpcontext = " About \"Photo!\" v." QUOTE(VERSION),
        .onPaint = NULL,
        .result = ACTION_ABOUT
    }
};
const menu_t MainMenu = {
    .x = 1, .y = 3, .width = 12, .height = 7,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = MainMenuItems, .last_item = LAST_ITEM(MainMenuItems),
    .onShow = NULL, .onHelpContext = onHelpMainMenu,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultMainMenu
};

uint8_t onTranslateSubResultMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    if (self->sub == &AboutMenu) {
        return (value == MENU_RESULT_OK) ? self->result : ACTION_NONE;
    }
    return value;
}
uint8_t onHelpMainMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, selection->helpcontext);
    return 0;
}

// Main Menu execute
uint8_t menu_main_execute() BANKED {
    uint8_t menu_result;
    do {
        switch (menu_result = menu_execute(&MainMenu, NULL, NULL)) {
            case ACTION_CAMERA:
                PLAY_SFX(sound_ok);
                CHANGE_STATE(state_camera);
                return STATE_CHANGED();
            case ACTION_GALLERY:
                PLAY_SFX(sound_ok);
                CHANGE_STATE(state_gallery);
                return STATE_CHANGED();         // don't refresh screen if state changed
            case ACTION_FLASHER:
                PLAY_SFX(sound_ok);
                CHANGE_STATE(state_flasher);
                return STATE_CHANGED();         // don't refresh screen if state changed
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
