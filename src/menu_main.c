#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "musicmanager.h"
#include "screen.h"
#include "states.h"
#include "globals.h"

#include "misc_assets.h"

#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_settings.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

uint8_t onShowAbout(const struct menu_t * self, uint8_t * param);
const menu_item_t AboutMenuItems[] = {
    {
        .prev = NULL, .next = NULL,
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 8, .ofs_y = 4, .width = 0,
        .caption = " OK ",
        .onPaint = NULL,
        .result = MENU_RESULT_OK
    }
};
const menu_t AboutMenu = {
    .x = 4, .y = 8, .width = 12, .height = 6,
    .items = AboutMenuItems,
    .onShow = onShowAbout, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};
uint8_t onShowAbout(const menu_t * self, uint8_t * param) {
    param;
    menu_text_out(self->x + 4, self->y + 1, 0, SOLID_WHITE, "This is a");
    menu_text_out(self->x + 1, self->y + 2, 0, SOLID_WHITE, "proof-of-concept");
    return 0;
}

uint8_t onHelpMainMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onTranslateSubResultMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t MainMenuItemCamera = {
    .prev = &MainMenuItemAbout,     .next = &MainMenuItemGallery,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 10,
    .caption = " Camera",
    .helpcontext = " Make your own pictures",
    .onPaint = NULL,
    .result = ACTION_CAMERA
};
const menu_item_t MainMenuItemGallery = {
    .prev = &MainMenuItemCamera,    .next = &MainMenuItemSettings,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 10,
    .caption = " Image gallery",
    .helpcontext = " View your image gallery",
    .onPaint = NULL,
    .result = ACTION_GALLERY
};
const menu_item_t MainMenuItemSettings = {
    .prev = &MainMenuItemGallery,    .next = &MainMenuItemAbout,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 10,
    .caption = " Settings",
    .helpcontext = " Edit program settings",
    .onPaint = NULL,
    .result = ACTION_SETTINGS
};
const menu_item_t MainMenuItemAbout = {
    .prev = &MainMenuItemSettings,   .next = &MainMenuItemCamera,
    .sub = &AboutMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 4, .width = 10, .flags = MENUITEM_TERM,
    .caption = " About",
    .helpcontext = " About PXLR-Studio " QUOTE(VERSION),
    .onPaint = NULL,
    .result = MENU_RESULT_OK
};
const menu_t MainMenu = {
    .x = 1, .y = 3, .width = 12, .height = 6,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &MainMenuItemCamera,
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
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, selection->helpcontext);
    return 0;
}

// Main Menu execute
uint8_t menu_main_execute() BANKED {
    switch (menu_execute(&MainMenu, NULL, NULL)) {
        case ACTION_CAMERA:
            music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
            CHANGE_STATE(state_camera);
            return STATE_CHANGED();
        case ACTION_GALLERY:
            music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
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
            music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
            break;
    }
    return FALSE;
}