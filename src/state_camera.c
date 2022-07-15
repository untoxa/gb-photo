#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gbcamera.h"
#include "musicmanager.h"
#include "systemhelpers.h"
#include "joy.h"
#include "screen.h"
#include "states.h"
#include "bankdata.h"

#include "globals.h"
#include "state_camera.h"

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

BANKREF(state_camera)

camera_mode_e camera_mode = camera_mode_manual;
trigger_mode_e trigger_mode = trigger_mode_abutton;
after_action_e after_action = after_action_save;

uint8_t image_live_preview = TRUE;

int8_t current_exposure = 14;
int8_t current_gain = 0;
int8_t current_zero_point = 1;
int8_t current_edge_mode = 0;
int8_t current_voltage_ref = 3;
int16_t voltage_out = 192;
uint8_t dithering = TRUE;
uint8_t ditheringHighLight = TRUE;
uint8_t current_contrast = 9;
uint8_t invertOutput = FALSE;
uint8_t edge_exclusive = TRUE;

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
static const table_value_t edge_modes[] = {
    { CAM04_EDGE_RATIO_050, "50%" }, { CAM04_EDGE_RATIO_075, "75%" }, { CAM04_EDGE_RATIO_100, "100%" },{ CAM04_EDGE_RATIO_125, "125%" },
    { CAM04_EDGE_RATIO_200, "200%" },{ CAM04_EDGE_RATIO_300, "300%" },{ CAM04_EDGE_RATIO_400, "400%" },{ CAM04_EDGE_RATIO_500, "500%" },
};
static const table_value_t voltage_refs[] = {
    { CAM04_VOLTAGE_REF_00, "0.0" }, { CAM04_VOLTAGE_REF_05, "0.5" }, { CAM04_VOLTAGE_REF_10, "1.0" }, { CAM04_VOLTAGE_REF_15, "1.5" },
    { CAM04_VOLTAGE_REF_20, "2.0" }, { CAM04_VOLTAGE_REF_25, "2.5" }, { CAM04_VOLTAGE_REF_30, "3.0" }, { CAM04_VOLTAGE_REF_35, "3.5" },
};


void display_last_seen(uint8_t restore) {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    uint8_t ypos = (camera_mode == camera_mode_manual) ? (IMAGE_DISPLAY_Y + 1) : IMAGE_DISPLAY_Y;
    screen_load_image(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, last_seen);
    if (restore) screen_restore_rect(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
}

void RENDER_CAM_REG_EDEXOPGAIN()  { CAM_REG_EDEXOPGAIN  = ((edge_exclusive) ? CAM01F_EDGEEXCL_V_ON : CAM01F_EDGEEXCL_V_OFF) | 0x60 | gains[current_gain].value; }
void RENDER_CAM_REG_EXPTIME()     { CAM_REG_EXPTIME     = exposures[current_exposure]; }
void RENDER_CAM_REG_EDRAINVVREF() { CAM_REG_EDRAINVVREF = edge_modes[current_edge_mode].value | ((invertOutput) ? CAM04F_INV : CAM04F_POS) | voltage_refs[current_voltage_ref].value; }
void RENDER_CAM_REG_ZEROVOUT()    { CAM_REG_ZEROVOUT    = zero_points[current_zero_point].value | TO_VOLTAGE_OUT(voltage_out); }
inline void RENDER_CAM_REG_DITHERPATTERN() { dither_pattern_apply(dithering, ditheringHighLight, current_contrast - 1); }

void camera_load_settings() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_EDRAINVVREF();
    RENDER_CAM_REG_ZEROVOUT();
    RENDER_CAM_REG_DITHERPATTERN();
}

static void refresh_screen() {
    screen_clear_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, SOLID_BLACK);
    display_last_seen(TRUE);
}

uint8_t ENTER_state_camera() BANKED {
    refresh_screen();

    // load some initial settings
    camera_load_settings();
    if (image_live_preview) image_capture();

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
    .id = idAssistedExposure,
    .caption = " %sms",
    .helpcontext = " Exposure time",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemAssistedContrast = {
    .prev = &CameraMenuItemAssistedExposure,    .next = &CameraMenuItemAssistedDither,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 0, .width = 5,
    .id = idContrast,
    .caption = " Cont: %d",
    .helpcontext = " Contrast level",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemAssistedDither = {
    .prev = &CameraMenuItemAssistedContrast,     .next = &CameraMenuItemAssistedDitherLight,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 0, .width = 5,
    .id = idDither,
    .caption = " %s",
    .helpcontext = " Dithering on/off",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemAssistedDitherLight = { // ToDo: remove this menu option when it's being set automatically via `.id = idAssistedExposure`
    .prev = &CameraMenuItemAssistedDither,     .next = &CameraMenuItemAssistedExposure,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 0, .width = 5, .flags = MENUITEM_TERM,
    .id = idDitherLight,
    .caption = " %s",
    .helpcontext = " Dithering light level",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};

const menu_t CameraMenuAssisted = {
    .x = 0, .y = 0, .width = 0, .height = 0,
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
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualGain = {
    .prev = &CameraMenuItemManualExposure,  .next = &CameraMenuItemManualVOut,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 0, .width = 5,
    .id = idGain,
    .caption = " %s",
    .helpcontext = " Sensor gain",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualVOut = {
    .prev = &CameraMenuItemManualGain,      .next = &CameraMenuItemManualItem3,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 0, .width = 5,
    .id = idVOut,
    .caption = " %dmv",
    .helpcontext = " Sensor voltage out",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualItem3 = {
    .prev = &CameraMenuItemManualVOut,      .next = &CameraMenuItemManualContrast,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 0, .width = 5,
    .caption = " Item 3",
    .helpcontext = " Some item 3",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualContrast = {
    .prev = &CameraMenuItemManualItem3,    .next = &CameraMenuItemManualDither,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 0, .ofs_y = 1, .width = 5,
    .id = idContrast,
    .caption = " Cont: %d",
    .helpcontext = " Contrast level",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualDither = {
    .prev = &CameraMenuItemManualContrast,     .next = &CameraMenuItemManualDitherLight,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 1, .width = 5,
    .id = idDither,
    .caption = " %s",
    .helpcontext = " Dithering on/off",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualDitherLight = {
    .prev = &CameraMenuItemManualDither,     .next = &CameraMenuItemInvertedOutput,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 1, .width = 5,
    .id = idDitherLight,
    .caption = " %s",
    .helpcontext = " Dithering light level",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemInvertedOutput = {
    .prev = &CameraMenuItemManualDitherLight,    .next = &CameraMenuItemManualZeroPoint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 1, .width = 5,
    .id = idInvOutput,
    .caption = " %s",
    .helpcontext = " Invert output",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualZeroPoint = {
    .prev = &CameraMenuItemInvertedOutput,  .next = &CameraMenuItemManualVoltRef,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 0, .ofs_y = 2, .width = 5,
    .id = idZeroPoint,
    .caption = " %s",
    .helpcontext = " Sensor zero point",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualVoltRef = {
    .prev = &CameraMenuItemManualZeroPoint, .next = &CameraMenuItemEdgeMode,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 5, .ofs_y = 2, .width = 5,
    .id = idVoltageRef,
    .caption = " %sv",
    .helpcontext = " Sensor voltage reference",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemEdgeMode = {
    .prev = &CameraMenuItemManualVoltRef,   .next = &CameraMenuItemManualEdgeExclusive,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 10, .ofs_y = 2, .width = 5,
    .id = idEdgeMode,
    .caption = " %s",
    .helpcontext = " Sensor edge mode",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_item_t CameraMenuItemManualEdgeExclusive = {
    .prev = &CameraMenuItemEdgeMode,        .next = &CameraMenuItemManualExposure,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 15, .ofs_y = 2, .width = 5, .flags = MENUITEM_TERM,
    .id = idEdgeExclusive,
    .caption = " %s",
    .helpcontext = "Sensor edge exclusive",
    .onPaint = onCameraMenuItemPaint,
    .result = ACTION_SHUTTER
};
const menu_t CameraMenuManual = {
    .x = 0, .y = 0, .width = 0, .height = 0,
    .items = &CameraMenuItemManualExposure,
    .onShow = NULL, .onIdle = onIdleCameraMenu, .onHelpContext = onHelpCameraMenu,
    .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};

uint8_t onTranslateKeyCameraMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    // swap J_UP/J_DOWN with J_LEFT/J_RIGHT buttons, because our menus are horizontal
    return (value & 0b11110000) | ((value << 1) & 0b00000100) | ((value >> 1) & 0b00000010) | ((value << 3) & 0b00001000) | ((value >> 3) & 0b00000001);
}
uint8_t onIdleCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu; selection;
    static change_direction_e change_direction;

    if (image_captured()) {
        display_last_seen(FALSE);
        if (image_live_preview) image_capture();
    }
    // select opens popup-menu
    if (KEY_PRESSED(J_SELECT)) {
        return ACTION_CAMERA_SUBMENU;
    } else if (KEY_PRESSED(J_START)) {
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
                if (redraw_selection = inc_dec_int8(&current_exposure, 1, 0, MAX_INDEX(exposures), change_direction)) RENDER_CAM_REG_EXPTIME();
                break;
            case idAssistedExposure:
                if (redraw_selection = inc_dec_int8(&current_exposure, 1, 0, MAX_INDEX(exposures), change_direction)) RENDER_CAM_REG_EXPTIME();
                // ToDo: Adjust other registers ("N", Edge Operation, Output Ref Voltage, Analog output gain) based on index of 'current_exposure'
                // ToDo: Adjust dither light level /High/Low) `->idDitherLight`
                break;
            case idGain:
                if (redraw_selection = inc_dec_int8(&current_gain, 1, 0, MAX_INDEX(gains), change_direction)) RENDER_CAM_REG_EDEXOPGAIN();
                break;
            case idVOut:
                if (redraw_selection = inc_dec_int16(&voltage_out, VOLTAGE_OUT_STEP, MIN_VOLTAGE_OUT, MAX_VOLTAGE_OUT, change_direction)) RENDER_CAM_REG_ZEROVOUT();
                break;
            case idDither:
                dithering = !dithering;
                RENDER_CAM_REG_DITHERPATTERN();
                break;
            case idDitherLight:
                ditheringHighLight = !ditheringHighLight;
                RENDER_CAM_REG_DITHERPATTERN();
                break;
            case idContrast:
                if (redraw_selection = inc_dec_int8(&current_contrast, 1, 1, NUM_CONTRAST_SETS, change_direction)) RENDER_CAM_REG_DITHERPATTERN();
                break;
            case idInvOutput:
                invertOutput = !invertOutput;
                RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idZeroPoint:
                if (redraw_selection = inc_dec_int8(&current_zero_point, 1, 0, MAX_INDEX(zero_points), change_direction)) RENDER_CAM_REG_ZEROVOUT();
                break;
            case idVoltageRef:
                if (redraw_selection = inc_dec_int8(&current_voltage_ref, 1, 0, MAX_INDEX(voltage_refs), change_direction)) RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idEdgeMode:
                if (redraw_selection = inc_dec_int8(&current_edge_mode, 1, 0, MAX_INDEX(edge_modes), change_direction)) RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idEdgeExclusive:
                edge_exclusive = !edge_exclusive;
                break;
            default:
                redraw_selection = FALSE;
                break;
        }
        // redraw selection if requested
        if (redraw_selection) menu_move_selection(menu, NULL, selection);
    }
    // wait for VBlank if not capturing (avoid HALT CPU state)
    if (!is_capturing()) wait_vbl_done();
    return 0;
}
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    static const uint8_t * const edgeOnOff[] = {"Edg Off", "Edg On"};
    static const uint8_t * const ditherOnOff[] = {"Dit Off", "Dit On"};
    static const uint8_t * const ditherHighLow[] = {"Dit Low", "Dit High"};
    static const uint8_t * const invert[] = {"Normal", "Inverted"};
    switch (self->id) {
        case idAssistedExposure:
        case idExposure: {
            uint16_t value = EXPOSURE_VALUE_TO_US(exposures[current_exposure]) / 100;
            uint8_t * buf = text_buffer + 100;
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
            sprintf(text_buffer, self->caption, gains[current_gain].caption);
            break;
        case idVOut:
            sprintf(text_buffer, self->caption, voltage_out);
            break;
        case idContrast:
            sprintf(text_buffer, self->caption, current_contrast);
            break;
        case idDither:
            sprintf(text_buffer, self->caption, ditherOnOff[((dithering) ? 1 : 0)]);
            break;
        case idDitherLight:
            sprintf(text_buffer, self->caption, ditherHighLow[((ditheringHighLight) ? 1 : 0)]);
            break;
        case idInvOutput:
            sprintf(text_buffer, self->caption, invert[((invertOutput) ? 1 : 0)]);
            break;
        case idZeroPoint:
            sprintf(text_buffer, self->caption, zero_points[current_zero_point].caption);
            break;
        case idVoltageRef:
            sprintf(text_buffer, self->caption, voltage_refs[current_voltage_ref].caption);
            break;
        case idEdgeMode:
            sprintf(text_buffer, self->caption, edge_modes[current_edge_mode].caption);
            break;
        case idEdgeExclusive:
            sprintf(text_buffer, self->caption, edgeOnOff[((edge_exclusive) ? 1 : 0)]);
            break;
        default:
            if (self->caption) strcpy(text_buffer, self->caption); else *text_buffer = 0;

    }
    return text_buffer;
}
uint8_t onHelpCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, 20, SOLID_BLACK, selection->helpcontext);
    return 0;
}

uint8_t UPDATE_state_camera() BANKED {
    static uint8_t menu_result;
    JOYPAD_RESET();
    switch (camera_mode) {
        case camera_mode_manual:
            menu_result = menu_execute(&CameraMenuManual, NULL, NULL);
            break;
        case camera_mode_assisted:
        case camera_mode_auto:
        case camera_mode_iterate:
            menu_result = menu_execute(&CameraMenuAssisted, NULL, NULL);
            break;
        default:
            // error, must not get here
            menu_result = ACTION_CAMERA_SUBMENU;
            break;
    }
    switch (menu_result) {
        case ACTION_SHUTTER:
            if (!is_capturing()) {
                music_play_sfx(BANK(shutter01), shutter01, SFX_MUTE_MASK(shutter01));
//                music_play_sfx(BANK(shutter02), shutter02, SFX_MUTE_MASK(shutter02));
                image_capture();
            }
            break;
        case ACTION_MAIN_MENU:
            if (!MainMenuDispatch(menu_execute(&MainMenu, NULL, NULL))) refresh_screen();
            break;
        case ACTION_CAMERA_SUBMENU: {
            switch (menu_result = menu_popup_camera_execute()) {
                case ACTION_MODE_MANUAL:
                case ACTION_MODE_ASSISTED:
                case ACTION_MODE_AUTO:
                case ACTION_MODE_ITERATE: {
                    static const camera_mode_e cmodes[] = {camera_mode_manual, camera_mode_assisted, camera_mode_auto, camera_mode_iterate};
                    camera_mode = cmodes[menu_result - ACTION_MODE_MANUAL];
                    break;
                }
                case ACTION_TRIGGER_ABUTTON:
                case ACTION_TRIGGER_TIMER:
                case ACTION_TRIGGER_INTERVAL: {
                    static const trigger_mode_e tmodes[] = {trigger_mode_abutton, trigger_mode_timer, trigger_mode_interval};
                    trigger_mode = tmodes[menu_result - ACTION_TRIGGER_ABUTTON];
                    break;
                }
                case ACTION_ACTION_SAVE:
                case ACTION_ACTION_PRINT:
                case ACTION_ACTION_SAVEPRINT: {
                    static const after_action_e aactions[] = {after_action_save, after_action_print, after_action_printsave};
                    after_action = aactions[menu_result - ACTION_ACTION_SAVE];
                    break;
                }
                case ACTION_RESTORE_DEFAULTS:
                    // TODO: restore defaults
                    break;
                default:
                    // error, must not get here
                    music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                    break;
            }
            refresh_screen();
            break;
        }
        default:
            music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
            break;
    }
    return 0;
}

uint8_t LEAVE_state_camera() BANKED {
    return 0;
}
