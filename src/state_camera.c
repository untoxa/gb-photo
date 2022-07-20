#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gbcamera.h"
#include "musicmanager.h"
#include "systemdetect.h"
#include "systemhelpers.h"
#include "joy.h"
#include "screen.h"
#include "states.h"
#include "bankdata.h"
#include "gbprinter.h"

#include "globals.h"
#include "state_camera.h"
#include "state_gallery.h"
#include "pic-n-rec.h"
#include "load_save.h"

#include "misc_assets.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"
#include "shutter01.h"
#include "shutter02.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_main.h"
#include "menu_popup_camera.h"

// dither patterns
#include "dither_patterns.h"

// frames
#include "print_frames.h"

BANKREF(state_camera)

camera_state_options_t camera_state;

uint8_t image_live_preview = TRUE;
uint8_t recording_video = FALSE;

camera_mode_settings_t current_settings[N_CAMERA_MODES];

uint8_t old_capture_reg = 0;    // old value for the captiring register (image ready detection)

static const uint16_t exposures[] = {
    US_TO_EXPOSURE_VALUE(200),    US_TO_EXPOSURE_VALUE(300),     US_TO_EXPOSURE_VALUE(400),    US_TO_EXPOSURE_VALUE(500),
    US_TO_EXPOSURE_VALUE(600),    US_TO_EXPOSURE_VALUE(800),     US_TO_EXPOSURE_VALUE(1000),   US_TO_EXPOSURE_VALUE(1250),
    US_TO_EXPOSURE_VALUE(1500),   US_TO_EXPOSURE_VALUE(2000),    US_TO_EXPOSURE_VALUE(2500),   US_TO_EXPOSURE_VALUE(3000),
    US_TO_EXPOSURE_VALUE(4000),   US_TO_EXPOSURE_VALUE(5000),    US_TO_EXPOSURE_VALUE(6000),   US_TO_EXPOSURE_VALUE(8000),
    US_TO_EXPOSURE_VALUE(10000),  US_TO_EXPOSURE_VALUE(12500),   US_TO_EXPOSURE_VALUE(15000),  US_TO_EXPOSURE_VALUE(20000),
    US_TO_EXPOSURE_VALUE(25000),  US_TO_EXPOSURE_VALUE(30000),   US_TO_EXPOSURE_VALUE(40000),  US_TO_EXPOSURE_VALUE(50000),
    US_TO_EXPOSURE_VALUE(60000),  US_TO_EXPOSURE_VALUE(70000),   US_TO_EXPOSURE_VALUE(80000),  US_TO_EXPOSURE_VALUE(100000),
    US_TO_EXPOSURE_VALUE(125000), US_TO_EXPOSURE_VALUE(160000),  US_TO_EXPOSURE_VALUE(200000), US_TO_EXPOSURE_VALUE(250000),
    US_TO_EXPOSURE_VALUE(300000), US_TO_EXPOSURE_VALUE(400000),  US_TO_EXPOSURE_VALUE(500000), US_TO_EXPOSURE_VALUE(600000),
    US_TO_EXPOSURE_VALUE(800000), US_TO_EXPOSURE_VALUE(1000000), US_TO_EXPOSURE_VALUE(1048560)
};
static const table_value_t gains[] = {
    { CAM01_GAIN_140, "14.0" }, { CAM01_GAIN_155, "15.5" }, { CAM01_GAIN_170, "17.0" }, { CAM01_GAIN_185, "18.5" },
    { CAM01_GAIN_200, "20.0" }, { CAM01_GAIN_215, "21.5" }, { CAM01_GAIN_230, "23.0" }, { CAM01_GAIN_245, "24.5" },
    { CAM01_GAIN_260, "26.0" }, { CAM01_GAIN_275, "27.5" }, { CAM01_GAIN_290, "29.0" }, { CAM01_GAIN_305, "30.5" },
    { CAM01_GAIN_320, "32.0" }, { CAM01_GAIN_350, "35.0" }, { CAM01_GAIN_380, "38.0" }, { CAM01_GAIN_410, "41.0" },
    { CAM01_GAIN_440, "44.0" }, { CAM01_GAIN_455, "45.5" }, { CAM01_GAIN_470, "47.0" }, { CAM01_GAIN_515, "51.5" },
    { CAM01_GAIN_575, "57.5" }
};
static const table_value_t zero_points[] = {
    { CAM05_ZERO_DIS, "None" }, { CAM05_ZERO_POS, "Positv" }, { CAM05_ZERO_NEG, "Negtv" }
};
static const table_value_t edge_ratios[] = {
    { CAM04_EDGE_RATIO_050, "50%" }, { CAM04_EDGE_RATIO_075, "75%" }, { CAM04_EDGE_RATIO_100, "100%" },{ CAM04_EDGE_RATIO_125, "125%" },
    { CAM04_EDGE_RATIO_200, "200%" },{ CAM04_EDGE_RATIO_300, "300%" },{ CAM04_EDGE_RATIO_400, "400%" },{ CAM04_EDGE_RATIO_500, "500%" },
};
static const table_value_t voltage_refs[] = {
    { CAM04_VOLTAGE_REF_00, "0.0" }, { CAM04_VOLTAGE_REF_05, "0.5" }, { CAM04_VOLTAGE_REF_10, "1.0" }, { CAM04_VOLTAGE_REF_15, "1.5" },
    { CAM04_VOLTAGE_REF_20, "2.0" }, { CAM04_VOLTAGE_REF_25, "2.5" }, { CAM04_VOLTAGE_REF_30, "3.0" }, { CAM04_VOLTAGE_REF_35, "3.5" },
};
static const table_value_t edge_operations[] = {
    { CAM01_EDGEOP_2D, "2D" }, { CAM01_EDGEOP_HORIZ, "Horiz" }, { CAM01_EDGEOP_VERT, "Vert" },{ CAM01_EDGEOP_NONE, "None" }
};


void display_last_seen(uint8_t restore) {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    uint8_t ypos = (OPTION(camera_mode) == camera_mode_manual) ? (IMAGE_DISPLAY_Y + 1) : IMAGE_DISPLAY_Y;
    screen_load_image(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, last_seen);
    if (restore) screen_restore_rect(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
}

void RENDER_CAM_REG_EDEXOPGAIN()  { CAM_REG_EDEXOPGAIN  = ((SETTING(edge_exclusive)) ? CAM01F_EDGEEXCL_V_ON : CAM01F_EDGEEXCL_V_OFF) | edge_operations[SETTING(edge_operation)].value | gains[SETTING(current_gain)].value; }
void RENDER_CAM_REG_EXPTIME()     { CAM_REG_EXPTIME     = exposures[SETTING(current_exposure)]; }
void RENDER_CAM_REG_EDRAINVVREF() { CAM_REG_EDRAINVVREF = edge_ratios[SETTING(current_edge_mode)].value | ((SETTING(invertOutput)) ? CAM04F_INV : CAM04F_POS) | voltage_refs[SETTING(current_voltage_ref)].value; }
void RENDER_CAM_REG_ZEROVOUT()    { CAM_REG_ZEROVOUT    = zero_points[SETTING(current_zero_point)].value | TO_VOLTAGE_OUT(SETTING(voltage_out)); }
inline void RENDER_CAM_REG_DITHERPATTERN() { dither_pattern_apply(SETTING(dithering), SETTING(ditheringHighLight), SETTING(current_contrast) - 1); }

void camera_load_settings() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_EDRAINVVREF();
    RENDER_CAM_REG_ZEROVOUT();
    RENDER_CAM_REG_DITHERPATTERN();
}

void camera_image_save() {
    // TODO: save last seen image to gallery
}

static void refresh_usage_indicator() {
    switch (OPTION(after_action)) {
        case after_action_picnrec_video:
            if (recording_video) strcpy(text_buffer, ICON_REC); else *text_buffer = 0;
            break;
        default:
            sprintf(text_buffer, "%hd/%hd", (uint8_t)images_taken(), (uint8_t)images_total());
            break;
    }
    menu_text_out(HELP_CONTEXT_WIDTH, 17, IMAGE_SLOTS_USED_WIDTH, SOLID_BLACK, text_buffer);
}

static void refresh_screen() {
    screen_clear_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, SOLID_BLACK);
    display_last_seen(TRUE);
    refresh_usage_indicator();
}

static uint8_t onPrinterProgress() BANKED {
    misc_render_progressbar(printer_completion, PRN_MAX_PROGRESS, text_buffer);
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, text_buffer);
    return 0;
}


uint8_t INIT_state_camera() BANKED {
    return 0;
}

uint8_t ENTER_state_camera() BANKED {
    CPU_SLOW(TRUE);
    refresh_screen();
    gbprinter_set_handler(onPrinterProgress, BANK(state_camera));

    // load some initial settings
    camera_load_settings();

    return 0;
}

// callback forward declarations
uint8_t onTranslateKeyCameraMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onIdleCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onHelpCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection);

// --- Assisted menu ---------------------------------
const menu_item_t CameraMenuItemAssistedExposure = {
    .prev = &CameraMenuItemAssistedDitherLight, .next = &CameraMenuItemAssistedContrast,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 0, .ofs_y = 0, .width = 5,
    .id = idExposure,
    .caption = " %sms",
    .helpcontext = " Exposure time",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemAssistedContrast = {
    .prev = &CameraMenuItemAssistedExposure,    .next = &CameraMenuItemAssistedDither,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 0, .width = 5,
    .id = idContrast,
    .caption = " " ICON_CONTRAST "\t%d",
    .helpcontext = " Contrast level",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemAssistedDither = {
    .prev = &CameraMenuItemAssistedContrast,     .next = &CameraMenuItemAssistedDitherLight,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 0, .width = 5,
    .id = idDither,
    .caption = " " ICON_DITHER "\t%s",
    .helpcontext = " Dithering on/off",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemAssistedDitherLight = { // ToDo: remove this menu option when it's being set automatically via `.id = idExposure`
    .prev = &CameraMenuItemAssistedDither,     .next = &CameraMenuItemAssistedExposure,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 0, .width = 5, .flags = MENUITEM_TERM,
    .id = idDitherLight,
    .caption = " " ICON_DITHER "\t%s",
    .helpcontext = " Dithering light level",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};

const menu_t CameraMenuAssisted = {
    .x = 0, .y = 0, .width = 0, .height = 0,
    .flags = MENU_INVERSE,
    .items = &CameraMenuItemAssistedExposure,
    .onShow = NULL, .onIdle = onIdleCameraMenu, .onHelpContext = onHelpCameraMenu,
    .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};

// --- Manual menu -----------------------------------
const menu_item_t CameraMenuItemManualExposure = {
    .prev = &CameraMenuItemManualEdgeExclusive, .next = &CameraMenuItemManualGain,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 0, .ofs_y = 0, .width = 5,
    .id = idExposure,
    .caption = " %sms",
    .helpcontext = " Exposure time",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualGain = {
    .prev = &CameraMenuItemManualExposure,      .next = &CameraMenuItemManualDither,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 0, .width = 5,
    .id = idGain,
    .caption = " " ICON_GAIN "\t%s",
    .helpcontext = " Sensor gain",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualDither = {
    .prev = &CameraMenuItemManualGain,          .next = &CameraMenuItemManualDitherLight,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 0, .width = 5,
    .id = idDither,
    .caption = " " ICON_DITHER "\t%s",
    .helpcontext = " Dithering on/off",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualDitherLight = {
    .prev = &CameraMenuItemManualDither,        .next = &CameraMenuItemManualContrast,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 0, .width = 5,
    .id = idDitherLight,
    .caption = " " ICON_DITHER "\t%s",
    .helpcontext = " Dithering light level",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualContrast = {
    .prev = &CameraMenuItemManualDitherLight,   .next = &CameraMenuItemManualZeroPoint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 0, .ofs_y = 1, .width = 5,
    .id = idContrast,
    .caption = " " ICON_CONTRAST "\t%d",
    .helpcontext = " Contrast level",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualZeroPoint = {
    .prev = &CameraMenuItemManualContrast,      .next = &CameraMenuItemManualVOut,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 1, .width = 5,
    .id = idZeroPoint,
    .caption = " %s",
    .helpcontext = " Sensor zero point",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualVOut = {
    .prev = &CameraMenuItemManualZeroPoint,     .next = &CameraMenuItemManualVoltRef,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 1, .width = 5,
    .id = idVOut,
    .caption = " %dmv",
    .helpcontext = " Sensor voltage out",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualVoltRef = {
    .prev = &CameraMenuItemManualVOut,          .next = &CameraMenuItemInvertedOutput,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 1, .width = 5,
    .id = idVoltageRef,
    .caption = " " ICON_VOLTAGE "\t%sv",
    .helpcontext = " Sensor voltage reference",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemInvertedOutput = {
    .prev = &CameraMenuItemManualVoltRef,       .next = &CameraMenuItemEdgeOperation,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 0, .ofs_y = 2, .width = 5,
    .id = idInvOutput,
    .caption = " %s",
    .helpcontext = " Invert output",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemEdgeOperation = {
    .prev = &CameraMenuItemInvertedOutput,      .next = &CameraMenuItemEdgeRatio,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 2, .width = 5,
    .id = idEdgeOperation,
    .caption = " " ICON_EDGE "\t%s",
    .helpcontext = " Sensor edge operation",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemEdgeRatio = {
    .prev = &CameraMenuItemEdgeOperation,   .next = &CameraMenuItemManualEdgeExclusive,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 2, .width = 5,
    .id = idEdgeRatio,
    .caption = " " ICON_EDGE "\t%s",
    .helpcontext = " Sensor edge ratio",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemManualEdgeExclusive = {
    .prev = &CameraMenuItemEdgeRatio,        .next = &CameraMenuItemManualExposure,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 2, .width = 5, .flags = MENUITEM_TERM,
    .id = idEdgeExclusive,
    .caption = " " ICON_EDGE "\t%s",
    .helpcontext = "Sensor edge exclusive",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_t CameraMenuManual = {
    .x = 0, .y = 0, .width = 0, .height = 0,
    .flags = MENU_INVERSE,
    .items = &CameraMenuItemManualExposure,
    .onShow = NULL, .onIdle = onIdleCameraMenu, .onHelpContext = onHelpCameraMenu,
    .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};
static const menu_item_t * last_menu_items[N_CAMERA_MODES] = { NULL, NULL, NULL, NULL };
uint8_t onTranslateKeyCameraMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    // swap J_UP/J_DOWN with J_LEFT/J_RIGHT buttons, because our menus are horizontal
    return (value & 0b11110000) | ((value << 1) & 0b00000100) | ((value >> 1) & 0b00000010) | ((value << 3) & 0b00001000) | ((value >> 3) & 0b00000001);
}
uint8_t onIdleCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu; selection;
    static change_direction_e change_direction;
    static uint8_t capture_triggered = FALSE;       // state of static variable persists between calls

    // save current selection
    last_menu_items[OPTION(camera_mode)] = selection;
    // process joypad buttons
    if (KEY_PRESSED(J_A)) {
        // A is a "shutter" button
        switch (OPTION(after_action)) {
            case after_action_picnrec_video:
                // toggle recording and start image capture
                recording_video = !recording_video;
                if (recording_video && !is_capturing()) image_capture();
                refresh_usage_indicator();
                break;
            default:
                if (!capture_triggered) {
                    music_play_sfx(BANK(shutter01), shutter01, SFX_MUTE_MASK(shutter01));
//                    music_play_sfx(BANK(shutter02), shutter02, SFX_MUTE_MASK(shutter02));
                    if (!is_capturing()) image_capture();
                    capture_triggered = TRUE;
                }
                break;
        }
    } else if (KEY_PRESSED(J_SELECT)) {
        // select opens popup-menu
        capture_triggered = FALSE;
        return ACTION_CAMERA_SUBMENU;
    } else if (KEY_PRESSED(J_START)) {
        // start open main menu
        capture_triggered = FALSE;
        return ACTION_MAIN_MENU;
    }
    // !!! d-pad keys are translated
    if (KEY_PRESSED(J_RIGHT)) change_direction = changeDecrease;
    else if (KEY_PRESSED(J_LEFT)) change_direction = changeIncrease;
    else change_direction = changeNone;

    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    if (change_direction != changeNone) {
        static uint8_t redraw_selection;
        redraw_selection = TRUE;
        // perform changes when pressing UP/DOWN while menu item with some ID is active
        switch (selection->id) {
            case idExposure:
                if (redraw_selection = inc_dec_int8(&SETTING(current_exposure), 1, 0, MAX_INDEX(exposures), change_direction)) RENDER_CAM_REG_EXPTIME();
                if (OPTION(camera_mode) == camera_mode_assisted) {
                    // ToDo: Adjust other registers ("N", Edge Operation, Output Ref Voltage, Analog output gain) based on index of 'current_exposure'
                    // ToDo: Adjust dither light level /High/Low) `->idDitherLight`
                }
                break;
            case idGain:
                if (redraw_selection = inc_dec_int8(&SETTING(current_gain), 1, 0, MAX_INDEX(gains), change_direction)) RENDER_CAM_REG_EDEXOPGAIN();
                break;
            case idVOut:
                if (redraw_selection = inc_dec_int16(&SETTING(voltage_out), VOLTAGE_OUT_STEP, MIN_VOLTAGE_OUT, MAX_VOLTAGE_OUT, change_direction)) RENDER_CAM_REG_ZEROVOUT();
                break;
            case idDither:
                SETTING(dithering) = !SETTING(dithering);
                RENDER_CAM_REG_DITHERPATTERN();
                break;
            case idDitherLight:
                SETTING(ditheringHighLight) = !SETTING(ditheringHighLight);
                RENDER_CAM_REG_DITHERPATTERN();
                break;
            case idContrast:
                if (redraw_selection = inc_dec_int8(&SETTING(current_contrast), 1, 1, NUM_CONTRAST_SETS, change_direction)) RENDER_CAM_REG_DITHERPATTERN();
                break;
            case idInvOutput:
                SETTING(invertOutput) = !SETTING(invertOutput);
                RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idZeroPoint:
                if (redraw_selection = inc_dec_int8(&SETTING(current_zero_point), 1, 0, MAX_INDEX(zero_points), change_direction)) RENDER_CAM_REG_ZEROVOUT();
                break;
            case idVoltageRef:
                if (redraw_selection = inc_dec_int8(&SETTING(current_voltage_ref), 1, 0, MAX_INDEX(voltage_refs), change_direction)) RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idEdgeRatio:
                if (redraw_selection = inc_dec_int8(&SETTING(current_edge_mode), 1, 0, MAX_INDEX(edge_ratios), change_direction)) RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idEdgeExclusive:
                SETTING(edge_exclusive) = !SETTING(edge_exclusive);
                break;
            case idEdgeOperation:
                if (redraw_selection = inc_dec_int8(&SETTING(edge_operation), 1, 0, MAX_INDEX(edge_operations), change_direction)) RENDER_CAM_REG_EDEXOPGAIN();
                break;
            default:
                redraw_selection = FALSE;
                break;
        }
        // redraw selection if requested
        if (redraw_selection) {
            save_camera_mode_settings(OPTION(camera_mode));
            menu_move_selection(menu, NULL, selection);
        }
    }

    // check image was captured, if yes, then restart capturing process
    if (image_captured()) {
        if (recording_video) picnrec_trigger();
        if (capture_triggered) {
            capture_triggered = FALSE;
            switch (OPTION(after_action)) {
                case after_action_picnrec:
                    picnrec_trigger();
                    break;
                case after_action_save:
                    camera_image_save();
                    refresh_usage_indicator();
                    break;
                case after_action_printsave:
                    camera_image_save();
                    refresh_usage_indicator();
                case after_action_print:
                    display_last_seen(FALSE);
                    return ACTION_CAMERA_PRINT;
                    break;
            }
        }
        display_last_seen(FALSE);
        if (image_live_preview || recording_video) image_capture();
    }

    // wait for VBlank if not capturing (avoid HALT CPU state)
    if (!is_capturing() && !recording_video) wait_vbl_done();
    return 0;
}
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    static const uint8_t * const on_off[] = {"Off", "On"};
    static const uint8_t * const low_high[] = {"Low", "High"};
    static const uint8_t * const norm_inv[] = {"Normal", "Inverted"};
    switch (self->id) {
        case idExposure: {
            uint16_t value = EXPOSURE_VALUE_TO_US(exposures[SETTING(current_exposure)]) / 100;
            uint8_t * buf = text_buffer_extra;
            uint8_t len = strlen(uitoa(value, buf, 10));
            if (len == 1) {
                *--buf = ',';
                *--buf = '0';
            } else {
                uint8_t * tail = buf + len - 1;
                len = *tail;
                if (len != '0') {
                    *tail++ = ',';
                    *tail++ = len;
                }
                *tail = 0;
            }
            sprintf(text_buffer, self->caption, buf);
            break;
        }
        case idGain:
            sprintf(text_buffer, self->caption, gains[SETTING(current_gain)].caption);
            break;
        case idVOut:
            sprintf(text_buffer, self->caption, SETTING(voltage_out));
            break;
        case idContrast:
            sprintf(text_buffer, self->caption, SETTING(current_contrast));
            break;
        case idDither:
            sprintf(text_buffer, self->caption, on_off[((SETTING(dithering)) ? 1 : 0)]);
            break;
        case idDitherLight:
            sprintf(text_buffer, self->caption, low_high[((SETTING(ditheringHighLight)) ? 1 : 0)]);
            break;
        case idInvOutput:
            sprintf(text_buffer, self->caption, norm_inv[((SETTING(invertOutput)) ? 1 : 0)]);
            break;
        case idZeroPoint:
            sprintf(text_buffer, self->caption, zero_points[SETTING(current_zero_point)].caption);
            break;
        case idVoltageRef:
            sprintf(text_buffer, self->caption, voltage_refs[SETTING(current_voltage_ref)].caption);
            break;
        case idEdgeRatio:
            sprintf(text_buffer, self->caption, edge_ratios[SETTING(current_edge_mode)].caption);
            break;
        case idEdgeExclusive:
            sprintf(text_buffer, self->caption, on_off[((SETTING(edge_exclusive)) ? 1 : 0)]);
            break;
        case idEdgeOperation:
            sprintf(text_buffer, self->caption, edge_operations[SETTING(edge_operation)].caption);
            break;
        default:
            if (self->caption) strcpy(text_buffer, self->caption); else *text_buffer = 0;
            break;
    }
    return text_buffer;
}
uint8_t onHelpCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, selection->helpcontext);
    return 0;
}

uint8_t UPDATE_state_camera() BANKED {
    static uint8_t menu_result;
    JOYPAD_RESET();
    // start capturing of the image
    if (image_live_preview) image_capture();
    // execute menu for the mode
    switch (OPTION(camera_mode)) {
        case camera_mode_manual:
            menu_result = menu_execute(&CameraMenuManual, NULL, last_menu_items[OPTION(camera_mode)]);
            break;
        case camera_mode_assisted:
        case camera_mode_auto:
        case camera_mode_iterate:
            menu_result = menu_execute(&CameraMenuAssisted, NULL, last_menu_items[OPTION(camera_mode)]);
            break;
        default:
            // error, must not get here
            menu_result = ACTION_CAMERA_SUBMENU;
            break;
    }
    switch (menu_result) {
        case ACTION_CAMERA_PRINT:
            remote_activate(REMOTE_DISABLED);
            CPU_FAST(TRUE);
            if (gbprinter_detect(10) == PRN_STATUS_OK) {
                gbprinter_print_image(last_seen, CAMERA_BANK_LAST_SEEN, print_frames + OPTION(print_frame_idx), BANK(print_frames));
            } else music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
            CPU_SLOW(TRUE);
            remote_activate(REMOTE_ENABLED);
            break;
        case ACTION_MAIN_MENU:
            recording_video = FALSE;
            if (!menu_main_execute()) refresh_screen();
            break;
        case ACTION_CAMERA_SUBMENU: {
            recording_video = FALSE;
            switch (menu_result = menu_popup_camera_execute()) {
                case ACTION_MODE_MANUAL:
                case ACTION_MODE_ASSISTED:
                case ACTION_MODE_AUTO:
                case ACTION_MODE_ITERATE: {
                    static const camera_mode_e cmodes[] = {camera_mode_manual, camera_mode_assisted, camera_mode_auto, camera_mode_iterate};
                    OPTION(camera_mode) = cmodes[menu_result - ACTION_MODE_MANUAL];
                    break;
                }
                case ACTION_TRIGGER_ABUTTON:
                case ACTION_TRIGGER_TIMER:
                case ACTION_TRIGGER_INTERVAL: {
                    static const trigger_mode_e tmodes[] = {trigger_mode_abutton, trigger_mode_timer, trigger_mode_interval};
                    OPTION(trigger_mode) = tmodes[menu_result - ACTION_TRIGGER_ABUTTON];
                    break;
                }
                case ACTION_ACTION_SAVE:
                case ACTION_ACTION_PRINT:
                case ACTION_ACTION_SAVEPRINT:
                case ACTION_ACTION_PICNREC:
                case ACTION_ACTION_PICNREC_VIDEO: {
                    static const after_action_e aactions[] = {after_action_save, after_action_print, after_action_printsave, after_action_picnrec, after_action_picnrec_video};
                    OPTION(after_action) = aactions[menu_result - ACTION_ACTION_SAVE];
                    break;
                }
                case ACTION_RESTORE_DEFAULTS:
                    restore_default_mode_settings(OPTION(camera_mode));
                    break;
                default:
                    // error, must not get here
                    music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                    break;
            }
            save_camera_state();
            refresh_screen();
            break;
        }
        default:
            music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
            break;
    }
    return FALSE;
}

uint8_t LEAVE_state_camera() BANKED {
    CPU_FAST(TRUE);
    recording_video = FALSE;
    gbprinter_set_handler(NULL, 0);
    return 0;
}
