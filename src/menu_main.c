#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "musicmanager.h"
#include "menus.h"
#include "menu_codes.h"
#include "screen.h"
#include "states.h"

#include "globals.h"

#include "menu_about.h"
#include "menu_debug.h"
#include "menu_yesno.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"

#if (DEBUG_ENABLED==1)
    #define MENUITEM_DEBUG &MainMenuItemDebug
    #define MAINMENU_HEIGHT 6
#else
    #define MENUITEM_DEBUG NULL
    #define MAINMENU_HEIGHT 5
#endif

#define Q(x) #x
#define QUOTE(x) Q(x)


uint8_t onHelpMainMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onTranslateSubResultMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t MainMenuItemCamera = {
    .prev = NULL,                   .next = &MainMenuItemGallery,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 10,
    .caption = " Camera",
    .helpcontext = " Make your own pictures",
    .onPaint = NULL,
    .result = ACTION_CAMERA
};
const menu_item_t MainMenuItemGallery = {
    .prev = &MainMenuItemCamera,    .next = &MainMenuItemAbout,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 10,
    .caption = " Image gallery",
    .helpcontext = " View your image gallery",
    .onPaint = NULL,
    .result = ACTION_GALLERY
};
const menu_item_t MainMenuItemAbout = {
    .prev = &MainMenuItemGallery,   .next = MENUITEM_DEBUG,
    .sub = &AboutMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 10,
    .caption = " About",
    .helpcontext = " About PXLR-Studio " QUOTE(VERSION),
    .onPaint = NULL,
    .result = 6
};
#if (DEBUG_ENABLED==1)
const menu_item_t MainMenuItemDebug = {
    .prev = &MainMenuItemAbout,     .next = NULL,
    .sub = &DebugMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 4, .width = 10,
    .caption = " Debug",
    .helpcontext = " Show debug info",
    .onPaint = NULL,
    .result = MENU_RESULT_CLOSE
};
#endif
const menu_t MainMenu = {
    .x = 1, .y = 3, .width = 12, .height = MAINMENU_HEIGHT,
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

// Main Menu dispatcher function
uint8_t MainMenuDispatch(uint8_t menu_result) {
    switch (menu_result) {
        case MENU_RESULT_CLOSE:
            // close menu: do nothing
            return FALSE;
        case ACTION_CAMERA:
            CHANGE_STATE(state_camera);
            return STATE_CHANGED();
        case ACTION_GALLERY:
            CHANGE_STATE(state_gallery);
            return STATE_CHANGED();         // don't refresh screen if state changed
        default:
            // default action
            music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
            break;
    }
    return FALSE;
}