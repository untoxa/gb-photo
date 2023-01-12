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
    screen_load_image_banked(self->x + ((self->width - (logo_about_WIDTH / logo_about_TILE_W)) >> 1), self->y, logo_about_WIDTH / logo_about_TILE_W, logo_about_HEIGHT / logo_about_TILE_H, logo_about_tiles, BANK(logo_about));
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
    return 0;
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
        .caption = " Settings",
        .helpcontext = " Edit program settings",
        .onPaint = NULL,
        .result = ACTION_SETTINGS
    }, {
        .sub = &AboutMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 10,
        .caption = " About",
        .helpcontext = " About Photo! " QUOTE(VERSION),
        .onPaint = NULL,
        .result = MENU_RESULT_OK
    }
};
const menu_t MainMenu = {
    .x = 1, .y = 3, .width = 12, .height = 6,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = MainMenuItems, .last_item = LAST_ITEM(MainMenuItems),
    .onShow = NULL, .onHelpContext = onHelpMainMenu,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultMainMenu
};

uint8_t onTranslateSubResultMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    if (menu == &YesNoMenu) {
        return (value == MENU_RESULT_YES) ? self->result : ACTION_NONE;
    }
    return value;
}
uint8_t onHelpMainMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, WHITE_ON_BLACK, selection->helpcontext);
    return 0;
}

// Main Menu execute
uint8_t menu_main_execute() BANKED {
    switch (menu_execute(&MainMenu, NULL, NULL)) {
        case ACTION_CAMERA:
            music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok), MUSIC_SFX_PRIORITY_MINIMAL);
            CHANGE_STATE(state_camera);
            return STATE_CHANGED();
        case ACTION_GALLERY:
            music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok), MUSIC_SFX_PRIORITY_MINIMAL);
            CHANGE_STATE(state_gallery);
            return STATE_CHANGED();         // don't refresh screen if state changed
        case ACTION_SETTINGS:
            menu_settings_execute();
            break;
        case MENU_RESULT_OK:
            // do nothing, no error sound
            break;
        default:
            // default action
            music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error), MUSIC_SFX_PRIORITY_MINIMAL);
            break;
    }
    return FALSE;
}
