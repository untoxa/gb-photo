#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "compat.h"
#include "musicmanager.h"
#include "screen.h"
#include "states.h"
#include "globals.h"
#include "bankdata.h"
#include "print_frames.h"
#include "load_save.h"
#include "palette.h"
#include "fade.h"
#include "joy.h"
#include "systemhelpers.h"
#include "ir.h"

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
    idPrintFrame,
    idSettingsPrintFrame,
    idSettingsPrintFast,
    idSettingsAltBorder,
    idSettingsCGBPalette,
    idSettingsShowGrid,
    idSettingsSaveConfirm,
    idSettingsIRRemoteShutter,
    idSettingsBootToCamera,
    idSettingsFlipImage,
    idSettingsDoubleSpeed,
    idSettingsDisplayExposure,
    idSettingsEnableDMA,
    idSettingsCartType
} settings_menu_e;


uint8_t onTranslateSubResultSettings(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onHelpSettings(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onShowSettings(const menu_t * self, uint8_t * param);
uint8_t onIdleSettings(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t * onSettingsMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onSettingsMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);

const menu_item_t FrameMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME0
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME1
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME2
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME3
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 5, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME4
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 6, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME5
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 7, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME6
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 8, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME7
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 9, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME8
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 10, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME9
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 11, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME10
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 12, .width = 8,
        .id = idPrintFrame,
        .caption = " %s",
        .helpcontext = " %s",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_PRINT_FRAME11
    }
};
const menu_t PrinterFramesMenu = {
    .x = 3, .y = 1, .width = 15, .height = LENGTH(FrameMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_OK,
    .items = FrameMenuItems, .last_item = LAST_ITEM(FrameMenuItems),
    .onShow = NULL, .onHelpContext = onHelpSettings,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

static uint8_t spinedit_palette_value = 0;
const spinedit_value_names_t PaletteSpinEditNames[] = {
    { .value = 0, .name = " Arctic\t\t" },
    { .value = 1, .name = " Cyan\t\t"   },
    { .value = 2, .name = " Thermal\t"  },
    { .value = 3, .name = " Circuits\t" },
    { .value = 4, .name = " Grape\t\t"  },
    { .value = 5, .name = " Japan\t\t"  },
    { .value = 6, .name = " Bavaria\t"  }
};
const spinedit_params_t PaletteSpinEditParams = {
    .caption = "Palette:",
    .min_value = 0,
    .max_value = MAX_INDEX(PaletteSpinEditNames),
    .value = &spinedit_palette_value,
    .names = PaletteSpinEditNames,
    .last_name = LAST_ITEM(PaletteSpinEditNames)
};
const uint8_t * const PaletteNames[] = { "Arctic", "Cyan", "Thermal", "Circuits", "Grape", "Japan", "Bavaria" };

static uint8_t spinedit_flip_value = camera_flip_none;
const spinedit_value_names_t FlipSpinEditNames[] = {
    { .value = camera_flip_none, .name = " None\t\t" },
    { .value = camera_flip_xy,   .name = " Flip XY\t" },
    { .value = camera_flip_x,    .name = " Flip X\t\t" }
};
const spinedit_params_t FlipSpinEditParams = {
    .caption = "Image flip:",
    .min_value = camera_flip_none,
    .max_value = MAX_INDEX(FlipSpinEditNames),
    .value = &spinedit_flip_value,
    .names = FlipSpinEditNames,
    .last_name = LAST_ITEM(FlipSpinEditNames)
};
const uint8_t * const FlipNames[] = { "None", "Flip XY", "Flip X" };

static uint8_t spinedit_carttype_value = 0;
const spinedit_value_names_t CartTypeSpinEditNames[] = {
    { .value = cart_type_HDR,    .name = " Generic\t" },
    { .value = cart_type_iG_AIO, .name = " iG AIO\t\t" }
};
const spinedit_params_t CartTypeSpinEditParams = {
    .caption = "Cart type:",
    .min_value = cart_type_HDR,
    .max_value = MAX_INDEX(CartTypeSpinEditNames),
    .value = &spinedit_carttype_value,
    .names = CartTypeSpinEditNames,
    .last_name = LAST_ITEM(CartTypeSpinEditNames)
};
const uint8_t * const CartTypeNames[] = { "Generic", "iG AIO" };

const menu_item_t SettingsMenuItems[] = {
    {
        .sub = &PrinterFramesMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 13,
        .id = idSettingsPrintFrame,
        .caption = " Frame\t\t[%s]",
        .helpcontext = " Select frame for printing",
        .onPaint = onSettingsMenuItemPaint,
        .result = MENU_RESULT_OK
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 13,
        .id = idSettingsShowGrid,
        .caption = " Show screen grid\t\t%s",
        .helpcontext = " Show grid on the live view",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_SHOW_GRID
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 13,
        .id = idSettingsSaveConfirm,
        .caption = " Save confirmation\t%s",
        .helpcontext = " Confirm saving of picture",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_SAVE_CONF
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 13,
        .id = idSettingsBootToCamera,
        .caption = " Quick boot\t\t\t%s",
        .helpcontext = " Boot into the camera mode",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_BOOT_TO_CAM
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&FlipSpinEditParams,
        .ofs_x = 1, .ofs_y = 5, .width = 13,
        .id = idSettingsFlipImage,
        .caption = " Flip image\t[%s]",
        .helpcontext = " Flip the live view image",
        .onPaint = onSettingsMenuItemPaint,
        .result = ACTION_SETTINGS_FLIP_IMAGE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 6, .width = 13,
        .id = idSettingsDisplayExposure,
        .caption = " Display exposure\t\t%s",
        .helpcontext = " Exposure display in auto mode",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_DISPLAY_EXP
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&CartTypeSpinEditParams,
        .ofs_x = 1, .ofs_y = 7, .width = 13,
        .id = idSettingsCartType,
        .caption = " Cart type\t[%s]",
        .helpcontext = " Select flash cart type",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_CARTTYPE
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&PaletteSpinEditParams,
        .ofs_x = 1, .ofs_y = 8, .width = 13,
        .id = idSettingsCGBPalette,
        .caption = " Palette\t[%s]",
        .helpcontext = " Select CGB palette",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_CGB_PALETTE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 9, .width = 13,
        .id = idSettingsPrintFast,
        .caption = " Fast printing\t\t\t%s",
        .helpcontext = " Enable CGB fast printing",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_PRINT_FAST
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 10, .width = 13,
        .id = idSettingsIRRemoteShutter,
        .caption = " IR remote\t\t\t\t%s",
        .helpcontext = " CGB IR sensor as shutter",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_IR_REMOTE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 11, .width = 13,
        .id = idSettingsDoubleSpeed,
        .caption = " Double speed\t\t\t%s",
        .helpcontext = " CGB double speed mode",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_DOUBLESPEED
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 12, .width = 13,
        .id = idSettingsEnableDMA,
        .caption = " Enable DMA\t\t\t%s",
        .helpcontext = " Enable CGB DMA transfers",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_ENABLE_DMA
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 13, .width = 13,
        .id = idSettingsAltBorder,
        .caption = " Alt. SGB border\t\t%s",
        .helpcontext = " Switch different SGB borders",
        .onPaint = onSettingsMenuItemPaint,
        .onGetProps = onSettingsMenuItemProps,
        .result = ACTION_SETTINGS_ALT_BORDER
    }
};
const menu_t GlobalSettingsMenu = {
    .x = 2, .y = 1, .width = 15, .height = LENGTH(SettingsMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = SettingsMenuItems, .last_item = LAST_ITEM(SettingsMenuItems),
    .onShow = onShowSettings, .onIdle = onIdleSettings, .onHelpContext = onHelpSettings,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultSettings
};
static const menu_item_t * settings_menu_last_selection;
static bool settings_menu_repaint;

uint8_t onTranslateSubResultSettings(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch (self->id) {
        case idSettingsCGBPalette:
        case idSettingsFlipImage:
        case idSettingsCartType:
            return (value == MENU_RESULT_YES) ? self->result : MENU_RESULT_OK;
        default:
            break;
    }
    return value;
}
uint8_t onHelpSettings(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    settings_menu_e menuid = (settings_menu_e)selection->id;
    if ((settings_menu_e)selection->id == idPrintFrame) {
        uint8_t frame_no = selection->result - ACTION_PRINT_FRAME_FIRST;
        frame_get_desc(text_buffer_extra, frame_no);
        sprintf(text_buffer, selection->helpcontext, text_buffer_extra);
        menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, ITEM_DEFAULT, text_buffer);
        frame_display_thumbnail(menu->x + 10, menu->y + 5, frame_no, BLACK_ON_WHITE);
    } else menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, ITEM_DEFAULT, selection->helpcontext);
    return 0;
}
uint8_t onShowSettings(const menu_t * self, uint8_t * param) {
    self; param;
    return (settings_menu_repaint) ? MENU_PROP_DEFAULT : MENU_PROP_SELECTION;
}
uint8_t onIdleSettings(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    settings_menu_last_selection = selection;

    sync_vblank();

    return 0;
}
uint8_t * onSettingsMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    static const uint8_t * checkbox[] = {ICON_CBX, ICON_CBX_CHECKED};
    settings_menu_e menuid = (settings_menu_e)self->id;
    switch (menuid) {
        case idPrintFrame:
            frame_get_caption(text_buffer_extra, self->result - ACTION_PRINT_FRAME_FIRST);
            sprintf(text_buffer, self->caption, text_buffer_extra);
            break;
        case idSettingsPrintFrame:
            frame_get_caption(text_buffer_extra, OPTION(print_frame_idx));
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
        case idSettingsSaveConfirm:
            sprintf(text_buffer, self->caption, checkbox[OPTION(save_confirm)]);
            break;
        case idSettingsIRRemoteShutter:
            sprintf(text_buffer, self->caption, checkbox[OPTION(ir_remote_shutter)]);
            break;
        case idSettingsBootToCamera:
            sprintf(text_buffer, self->caption, checkbox[OPTION(boot_to_camera_mode)]);
            break;
        case idSettingsFlipImage:
            sprintf(text_buffer, self->caption, FlipNames[OPTION(flip_live_view)]);
            break;
        case idSettingsDoubleSpeed:
            sprintf(text_buffer, self->caption, checkbox[OPTION(double_speed)]);
            break;
        case idSettingsDisplayExposure:
            sprintf(text_buffer, self->caption, checkbox[OPTION(display_exposure)]);
            break;
        case idSettingsEnableDMA:
            sprintf(text_buffer, self->caption, checkbox[OPTION(enable_DMA)]);
            break;
        case idSettingsCartType:
            sprintf(text_buffer, self->caption, CartTypeNames[OPTION(cart_type)]);
            break;
        default:
            *text_buffer = 0;
            break;
    }
    return text_buffer;
}
uint8_t onSettingsMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    switch ((settings_menu_e)self->id) {
        case idSettingsDoubleSpeed:
            return ((!_is_COLOR) || (OPTION(cart_type) == cart_type_iG_AIO)) ? ITEM_DISABLED : ITEM_DEFAULT;
        case idSettingsPrintFast:
        case idSettingsCGBPalette:
        case idSettingsIRRemoteShutter:
        case idSettingsEnableDMA:
            return (_is_COLOR) ? ITEM_DEFAULT : ITEM_DISABLED;
        case idSettingsAltBorder:
            return (_is_SUPER) ? ITEM_DEFAULT : ITEM_DISABLED;
        default:
            return ITEM_DEFAULT;
    }
}

void menu_settings_execute(void) BANKED {
    uint8_t menu_result;
    settings_menu_repaint = true;
    settings_menu_last_selection = NULL;
    spinedit_palette_value = OPTION(cgb_palette_idx);
    spinedit_carttype_value = OPTION(cart_type);
    spinedit_flip_value = OPTION(flip_live_view);
    do {
        menu_result = menu_execute(&GlobalSettingsMenu, NULL, settings_menu_last_selection), settings_menu_repaint = false;
        if ((menu_result >= ACTION_PRINT_FRAME_FIRST) && (menu_result <= ACTION_PRINT_FRAME_LAST)) {
            OPTION(print_frame_idx) = (menu_result - ACTION_PRINT_FRAME_FIRST);
            save_camera_state();
            settings_menu_repaint = true;
        } else {
            switch (menu_result) {
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
                        palette_apply();
                    }
                    settings_menu_repaint = true;
                    break;
                case ACTION_SETTINGS_SHOW_GRID:
                    OPTION(show_grid) = !OPTION(show_grid);
                    save_camera_state();
                    break;
                case ACTION_SETTINGS_SAVE_CONF:
                    OPTION(save_confirm) = !OPTION(save_confirm);
                    save_camera_state();
                    break;
                case ACTION_SETTINGS_IR_REMOTE:
                    OPTION(ir_remote_shutter) = !OPTION(ir_remote_shutter);
                    save_camera_state();
                    if (_is_COLOR) {
                        // Apply change immediately in camera state, otherwise will be set entering camera state
                        if (CURRENT_PROGRAM_STATE == state_camera) {
                            if (OPTION(ir_remote_shutter)) {
                                ir_sense_start();
                            } else {
                                ir_sense_stop();
                            }
                        }
                    }
                    break;
                case ACTION_SETTINGS_BOOT_TO_CAM:
                    OPTION(boot_to_camera_mode) = !OPTION(boot_to_camera_mode);
                    save_camera_state();
                    break;
                case ACTION_SETTINGS_FLIP_IMAGE:
                    OPTION(flip_live_view) = (camera_flip_e)spinedit_flip_value;
                    save_camera_state();
                    settings_menu_repaint = true;
                    break;
                case ACTION_SETTINGS_DOUBLESPEED:
                    OPTION(double_speed) = !OPTION(double_speed);
                    if (_is_COLOR) {
                        fade_out_modal();
                        if (OPTION(double_speed)) {
                            CPU_FAST();
                            if ((OPTION(after_action) == after_action_picnrec) || (OPTION(after_action) == after_action_picnrec_video)) OPTION(after_action) = after_action_save;
                        } else CPU_SLOW();
                        music_setup_timer_ex(_is_CPU_FAST);
                        fade_in_modal();
                    }
                    save_camera_state();
                    break;
                case ACTION_SETTINGS_DISPLAY_EXP:
                    OPTION(display_exposure) = !OPTION(display_exposure);
                    save_camera_state();
                    break;
                case ACTION_SETTINGS_ENABLE_DMA:
                    OPTION(enable_DMA) = !OPTION(enable_DMA);
                    save_camera_state();
                    break;
                case ACTION_SETTINGS_CARTTYPE:
                    OPTION(cart_type) = (cart_type_e)spinedit_carttype_value;
                    if (OPTION(cart_type) == cart_type_iG_AIO) {
                        OPTION(double_speed) = false;
                        if (_is_COLOR) {
                            fade_out_modal();
                            CPU_SLOW();
                            music_setup_timer_ex(_is_CPU_FAST);
                            fade_in_modal();
                        }
                    } else {
                        if ((OPTION(after_action) == after_action_picnrec) || (OPTION(after_action) == after_action_picnrec_video)) OPTION(after_action) = after_action_save;
                    }
                    save_camera_state();
                    settings_menu_repaint = true;
                    break;
                case MENU_RESULT_OK:
                    settings_menu_repaint = true;
                default:
                    // unknown command or cancel
                    PLAY_SFX(sound_ok);
                    break;
            }
        }
    } while (menu_result != ACTION_NONE);
}
