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
#include "palette.h"
#include "fade_manager.h"
#include "joy.h"
#include "systemhelpers.h"

#include "state_camera.h"

#include "misc_assets.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_settings.h"
#include "menu_spinedit.h"

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
    idSettingsAltBorder,
    idSettingsCGBPalette,
    idSettingsShowGrid
} settings_menu_e;


uint8_t onTranslateSubResultSettings(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onHelpSettings(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t * onSettingsMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);

const menu_item_t FrameMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 8,
        .id = idPrintFrame0,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME0
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 8,
        .id = idPrintFrame1,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME1
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 8,
        .id = idPrintFrame2,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME2
    }
};
const menu_t PrinterFramesMenu = {
    .x = 7, .y = 4, .width = 10, .height = 5,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = FrameMenuItems, .last_item = LAST_ITEM(FrameMenuItems),
    .onShow = NULL, .onHelpContext = onHelpSettings,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

static uint8_t spinedit_palette_value = 0;
const spinedit_value_names_t PaletteSpinEditNames[] = {
    { .next = PaletteSpinEditNames + 1, .value = 0, .name = " Arctic\t\t" },
    { .next = PaletteSpinEditNames + 2, .value = 1, .name = " Cyan\t\t"   },
    { .next = PaletteSpinEditNames + 3, .value = 2, .name = " Thermal\t"  },
    { .next = PaletteSpinEditNames + 4, .value = 3, .name = " Circuits\t" },
    { .next = PaletteSpinEditNames + 5, .value = 4, .name = " Grape\t\t"  },
    { .next = PaletteSpinEditNames + 6, .value = 5, .name = " Japan\t\t"  },
    { .next = NULL,                     .value = 6, .name = " Bavaria\t"  }
};
const spinedit_params_t PaletteSpinEditParams = {
    .caption = "Palette:",
    .min_value = 0,
    .max_value = MAX_INDEX(PaletteSpinEditNames),
    .value = &spinedit_palette_value,
    .names = PaletteSpinEditNames
};
const uint8_t * const PaletteNames[] = { "Arctic", "Cyan", "Thermal", "Circuits", "Grape", "Japan", "Bavaria" };

const menu_item_t SettingsMenuItems[] = {
    {
        .sub = &PrinterFramesMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 13,
        .id = idSettingsPrintFrame,
        .caption = " Frame\t\t[%s]",
        .helpcontext = " Select frame for printing",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 13,
        .id = idSettingsPrintFast,
        .caption = " Fast printing\t\t\t%s",
        .helpcontext = " Enable CGB 32Kb/s printing",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_PRINT_FAST
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 13,
        .id = idSettingsAltBorder,
        .caption = " Alt. SGB border\t\t%s",
        .helpcontext = " Switch different SGB borders",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_ALT_BORDER
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&PaletteSpinEditParams,
        .ofs_x = 1, .ofs_y = 4, .width = 13,
        .id = idSettingsCGBPalette,
        .caption = " Palette\t[%s]",
        .helpcontext = " Select CGB palette",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_CGB_PALETTE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 5, .width = 13,
        .id = idSettingsShowGrid,
        .caption = " Show screen grid\t\t%s",
        .helpcontext = " Show grid on the live view",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_SHOW_GRID
    }
};
const menu_t GlobalSettingsMenu = {
    .x = 3, .y = 5, .width = 15, .height = 7,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = SettingsMenuItems, .last_item = LAST_ITEM(SettingsMenuItems),
    .onShow = NULL, .onHelpContext = onHelpSettings,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultSettings
};

uint8_t onTranslateSubResultSettings(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch (self->id) {
        case idSettingsCGBPalette:
            return (value == MENU_RESULT_YES) ? self->result : ACTION_NONE;
        default:
            break;
    }
    return value;
}
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
        case idSettingsCGBPalette:
            sprintf(text_buffer, self->caption, PaletteNames[OPTION(cgb_palette_idx)]);
            break;
        case idSettingsShowGrid:
            sprintf(text_buffer, self->caption, checkbox[OPTION(show_grid)]);
            break;
        default:
            *text_buffer = 0;
            break;
    }
    return text_buffer;
}

void menu_settings_execute() BANKED {
    uint8_t menu_result;
    spinedit_palette_value = OPTION(cgb_palette_idx);
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
        case ACTION_SETTINGS_CGB_PALETTE:
            OPTION(cgb_palette_idx) = spinedit_palette_value;
            save_camera_state();
            if (_is_COLOR) {
                palette_reload();
                fade_apply_palette_change_color(FADED_IN_FRAME);
            }
            break;
        case ACTION_SETTINGS_SHOW_GRID:
            OPTION(show_grid) = !OPTION(show_grid);
            save_camera_state();
            break;
        default:
            music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error), MUSIC_SFX_PRIORITY_MINIMAL);
            break;
    }
}
