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
    #define MAINMENU_HEIGHT 8
#else 
    #define MENUITEM_DEBUG NULL
    #define MAINMENU_HEIGHT 7
#endif

uint8_t onTranslateSubResultMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t MainMenuItemManual = {
    .prev = NULL,                   .next = &MainMenuItemBurst, 
    .sub = NULL, .sub_params = NULL,        
    .ofs_x = 1, .ofs_y = 1, .width = 10, 
    .caption = " Manual mode",
    .onPaint = NULL,
    .result = ACTION_MANUAL
};
const menu_item_t MainMenuItemBurst = {
    .prev = &MainMenuItemManual,    .next = &MainMenuItemAssisted,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 10, 
    .caption = " Burst mode",
    .onPaint = NULL,
    .result = ACTION_BURST
};
const menu_item_t MainMenuItemAssisted = {
    .prev = &MainMenuItemBurst,     .next = &MainMenuItemGallery,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 10, 
    .caption = " Assisted mode",
    .onPaint = NULL,
    .result = ACTION_ASSISTED
};
const menu_item_t MainMenuItemGallery = {
    .prev = &MainMenuItemAssisted,  .next = &MainMenuItemAbout,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 4, .width = 10,
    .caption = " Image gallery",
    .onPaint = NULL,
    .result = ACTION_GALLERY
};
const menu_item_t MainMenuItemAbout = {
    .prev = &MainMenuItemGallery,   .next = MENUITEM_DEBUG,
    .sub = &AboutMenu, .sub_params = NULL, 
    .ofs_x = 1, .ofs_y = 5, .width = 10,
    .caption = " About",
    .onPaint = NULL,
    .result = 6
};
#if (DEBUG_ENABLED==1)
const menu_item_t MainMenuItemDebug = {
    .prev = &MainMenuItemAbout,     .next = NULL,
    .sub = &DebugMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 6, .width = 10,
    .caption = " Debug",
    .onPaint = NULL,
    .result = MENU_RESULT_CLOSE
};
#endif
const menu_t MainMenu = {
    .x = 1, .y = 3, .width = 12, .height = MAINMENU_HEIGHT,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &MainMenuItemManual, 
    .onShow = NULL, .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultMainMenu
};

uint8_t onTranslateSubResultMainMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    if (menu == &YesNoMenu) {
        return (value == MENU_RESULT_YES) ? self->result : ACTION_NONE;
    }
    return value;
}

// Main Menu dispatcher function
uint8_t MainMenuDispatch(uint8_t menu_result) {
    switch (menu_result) {
        case MENU_RESULT_CLOSE:
            // close menu: do nothing
            return FALSE;
        case ACTION_MANUAL:
            CHANGE_STATE(state_shoot_manual);
            return STATE_CHANGED();
        case ACTION_BURST:
            CHANGE_STATE(state_shoot_burst);
            return STATE_CHANGED();
        case ACTION_ASSISTED:
            CHANGE_STATE(state_shoot_assisted);
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