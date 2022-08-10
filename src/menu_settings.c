#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "musicmanager.h"
#include "screen.h"
#include "states.h"
#include "globals.h"
#include "bankdata.h"
#include "print_frames.h"
#include "load_save.h"

#include "state_camera.h"

#include "misc_assets.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_settings.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"

typedef enum {
    idSettingsNone = 0,
    idPrintFrame0,
    idPrintFrame1,
    idPrintFrame2,
    idSettingsPrintFrame,
    idSettingsPrintFast,
    idSettingsAltBorder
} settings_menu_e;


uint8_t onHelpSettings(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t * onSettingsMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);

const menu_item_t FrameMenuItemNoFrame = {
    .prev = &FrameMenuItemWild,     .next = &FrameMenuItemPxlr,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .id = idPrintFrame0,
    .caption = " %s",
    .helpcontext = " %s",
    .onPaint = onSettingsMenuItemPaint,
    .result = ACTION_PRINT_FRAME0
};
const menu_item_t FrameMenuItemPxlr = {
    .prev = &FrameMenuItemNoFrame,  .next = &FrameMenuItemWild,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .id = idPrintFrame1,
    .caption = " %s",
    .helpcontext = " %s",
    .onPaint = onSettingsMenuItemPaint,
    .result = ACTION_PRINT_FRAME1
};
const menu_item_t FrameMenuItemWild = {
    .prev = &FrameMenuItemPxlr,     .next = &FrameMenuItemNoFrame,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 8, .flags = MENUITEM_TERM,
    .id = idPrintFrame2,
    .caption = " %s",
    .helpcontext = " %s",
    .onPaint = onSettingsMenuItemPaint,
    .result = ACTION_PRINT_FRAME2
};
const menu_t PrinterFramesMenu = {
    .x = 7, .y = 4, .width = 10, .height = 5,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &FrameMenuItemNoFrame,
    .onShow = NULL, .onHelpContext = onHelpSettings,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


const menu_item_t SettingsMenuItemPrintFrame = {
    .prev = &SettingsMenuItemAltBorder,     .next = &SettingsMenuItemPrintFast,
    .sub = &PrinterFramesMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 13,
    .id = idSettingsPrintFrame,
    .caption = " Frame\t[%s]",
    .helpcontext = " Select frame for printing",
    .onPaint = onSettingsMenuItemPaint,
    .result = ACTION_NONE
};
const menu_item_t SettingsMenuItemPrintFast = {
    .prev = &SettingsMenuItemPrintFrame,    .next = &SettingsMenuItemAltBorder,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 13,
    .id = idSettingsPrintFast,
    .caption = " Fast printing\t\t\t%s",
    .helpcontext = " Enable CGB fast transfer",
    .onPaint = onSettingsMenuItemPaint,
    .result = ACTION_SETTINGS_PRINT_FAST
};
const menu_item_t SettingsMenuItemAltBorder = {
    .prev = &SettingsMenuItemPrintFast,     .next = &SettingsMenuItemPrintFrame,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 13, .flags = MENUITEM_TERM,
    .id = idSettingsAltBorder,
    .caption = " Alt. SGB border\t\t%s",
    .helpcontext = " Switch different SGB borders",
    .onPaint = onSettingsMenuItemPaint,
    .result = ACTION_SETTINGS_ALT_BORDER
};
const menu_t GlobalSettingsMenu = {
    .x = 3, .y = 5, .width = 15, .height = 5,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &SettingsMenuItemPrintFrame,
    .onShow = NULL, .onHelpContext = onHelpSettings,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t onHelpSettings(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    switch ((settings_menu_e)selection->id) {
        case idPrintFrame0:
        case idPrintFrame1:
        case idPrintFrame2:
            banked_strcpy(text_buffer_extra, print_frames[(selection->id - idPrintFrame0)].desc, BANK(print_frames));
            sprintf(text_buffer, selection->helpcontext, text_buffer_extra);
            menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, text_buffer);
            break;
        default:
            menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, selection->helpcontext);
            break;
    }
    return 0;
}
uint8_t * onSettingsMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    static const uint8_t * checkbox[] = {ICON_CBX, ICON_CBX_CHECKED};
    switch ((settings_menu_e)self->id) {
        case idPrintFrame0:
        case idPrintFrame1:
        case idPrintFrame2:
            banked_strcpy(text_buffer_extra, print_frames[(self->id - idPrintFrame0)].caption, BANK(print_frames));
            sprintf(text_buffer, self->caption, text_buffer_extra);
            break;
        case idSettingsPrintFrame:
            banked_strcpy(text_buffer_extra, print_frames[OPTION(print_frame_idx)].caption, BANK(print_frames));
            sprintf(text_buffer, self->caption, text_buffer_extra);
            break;
        case idSettingsPrintFast:
            sprintf(text_buffer, self->caption, checkbox[OPTION(print_fast)]);
            break;
        case idSettingsAltBorder:
            sprintf(text_buffer, self->caption, checkbox[OPTION(fancy_sgb_border)]);
            break;
        default:
            *text_buffer = 0;
            break;
    }
    return text_buffer;
}

void menu_settings_execute() BANKED {
    uint8_t menu_result;
    switch (menu_result = menu_execute(&GlobalSettingsMenu, NULL, NULL)) {
        case ACTION_PRINT_FRAME0:
        case ACTION_PRINT_FRAME1:
        case ACTION_PRINT_FRAME2:
            OPTION(print_frame_idx) = (menu_result - ACTION_PRINT_FRAME0);
            save_camera_state();
            break;
        case ACTION_SETTINGS_PRINT_FAST:
            OPTION(print_fast) = !OPTION(print_fast);
            save_camera_state();
            break;
        case ACTION_SETTINGS_ALT_BORDER:
            OPTION(fancy_sgb_border) = !OPTION(fancy_sgb_border);
            save_camera_state();
            break;
        default:
            music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error), MUSIC_SFX_PRIORITY_MINIMAL);
            break;
    }
}
