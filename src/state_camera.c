#pragma bank 255

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdbool.h>
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
#include "linkcable.h"
#include "fade_manager.h"
#include "vector.h"
#include "counter.h"
#include "protected.h"
#include "histogram.h"
#include "math.h"
#include "scrollbar.h"

#include "globals.h"
#include "state_camera.h"
#include "state_gallery.h"
#include "pic-n-rec.h"
#include "load_save.h"

#include "misc_assets.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"
#include "sound_menu_alter.h"
#include "sound_timer.h"
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
uint8_t camera_do_shutter = FALSE;

COUNTER_DECLARE(camera_shutter_timer, uint8_t, 0);
COUNTER_DECLARE(camera_repeat_counter, uint8_t, 0);

camera_mode_settings_t current_settings[N_CAMERA_MODES];

camera_shadow_regs_t SHADOW;        // camera shadow registers for reading

#define SS_BRIGHTNESS_X 1
#define SS_BRIGHTNESS_Y 2
#define SS_BRIGHTNESS_LEN 14
static scrollbar_t ss_brightness;

#define SS_CONTRAST_X 2
#define SS_CONTRAST_Y 16
#define SS_CONTRAST_LEN 16
static scrollbar_t ss_contrast;

static const uint16_t exposures[] = {
    TO_EXPOSURE_VALUE(208),  // does this setting actually work on the real hardware?
    TO_EXPOSURE_VALUE(304),     TO_EXPOSURE_VALUE(400),     TO_EXPOSURE_VALUE(512),     TO_EXPOSURE_VALUE(608),
    TO_EXPOSURE_VALUE(800),     TO_EXPOSURE_VALUE(1008),    TO_EXPOSURE_VALUE(1264),    TO_EXPOSURE_VALUE(1504),
    TO_EXPOSURE_VALUE(2000),    TO_EXPOSURE_VALUE(2512),    TO_EXPOSURE_VALUE(3008),    TO_EXPOSURE_VALUE(4000),
    TO_EXPOSURE_VALUE(5008),    TO_EXPOSURE_VALUE(6000),    TO_EXPOSURE_VALUE(8000),    TO_EXPOSURE_VALUE(10000),
    TO_EXPOSURE_VALUE(12512),   TO_EXPOSURE_VALUE(15008),   TO_EXPOSURE_VALUE(20000),   TO_EXPOSURE_VALUE(25008),
    TO_EXPOSURE_VALUE(30000),   TO_EXPOSURE_VALUE(40000),   TO_EXPOSURE_VALUE(50000),   TO_EXPOSURE_VALUE(60000),
    TO_EXPOSURE_VALUE(70000),   TO_EXPOSURE_VALUE(80000),   TO_EXPOSURE_VALUE(100000),  TO_EXPOSURE_VALUE(125008),
    TO_EXPOSURE_VALUE(160000),  TO_EXPOSURE_VALUE(200000),  TO_EXPOSURE_VALUE(250000),  TO_EXPOSURE_VALUE(300000),
    TO_EXPOSURE_VALUE(400000),  TO_EXPOSURE_VALUE(500000),  TO_EXPOSURE_VALUE(600000),  TO_EXPOSURE_VALUE(800000),
    TO_EXPOSURE_VALUE(1000000), TO_EXPOSURE_VALUE(1048560)
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

void RENDER_CAM_REG_EDEXOPGAIN()  { SHADOW.CAM_REG_EDEXOPGAIN  = CAM_REG_EDEXOPGAIN  = ((SETTING(edge_exclusive)) ? CAM01F_EDGEEXCL_V_ON : CAM01F_EDGEEXCL_V_OFF) | edge_operations[SETTING(edge_operation)].value | gains[SETTING(current_gain)].value; }
void RENDER_CAM_REG_EXPTIME()     { SHADOW.CAM_REG_EXPTIME     = CAM_REG_EXPTIME     = swap_bytes(SETTING(current_exposure)); }
void RENDER_CAM_REG_EDRAINVVREF() { SHADOW.CAM_REG_EDRAINVVREF = CAM_REG_EDRAINVVREF = edge_ratios[SETTING(current_edge_ratio)].value | ((SETTING(invertOutput)) ? CAM04F_INV : CAM04F_POS) | voltage_refs[SETTING(current_voltage_ref)].value; }
void RENDER_CAM_REG_ZEROVOUT()    { SHADOW.CAM_REG_ZEROVOUT    = CAM_REG_ZEROVOUT    = zero_points[SETTING(current_zero_point)].value | TO_VOLTAGE_OUT(SETTING(voltage_out)); }
inline void RENDER_CAM_REG_DITHERPATTERN() { dither_pattern_apply(SETTING(dithering), SETTING(ditheringHighLight), SETTING(current_contrast) - 1); }

void RENDER_CAM_REGISTERS() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_EDRAINVVREF();
    RENDER_CAM_REG_ZEROVOUT();
    RENDER_CAM_REG_DITHERPATTERN();
}

void RENDER_REGS_FROM_EXPOSURE() {
    // Gain 32.0 | vRef 0.480 | No edge Operation    | Exposure time range from 1048ms to 394ms
    // Gain 26.0 | vRef 0.416 | 2-D edge mode        | Exposure time range from  573ms to 164ms
    // Gain 20.0 | vRef 0.224 | 2-D edge mode        | Exposure time range from  282ms to  32ms
    // Gain 14.0 | vRef 0.192 | 2-D edge mode        | Exposure time range from   67ms to 0.8ms
    // Gain 14.0 | vRef 0.160 | Horizontal edge mode | Exposure time range from  0.5ms to 0.3ms
    uint8_t apply_dither;
    uint16_t exposure = SETTING(current_exposure);
    if (exposure < TO_EXPOSURE_VALUE(512)) {
        SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
        SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
        SETTING(voltage_out)        = 160;
        SETTING(current_gain)       = 0;        // CAM01_GAIN_140
        if (apply_dither = (SETTING(ditheringHighLight)))
            SETTING(ditheringHighLight) = false;// dither HIGH
    } else if (exposure < TO_EXPOSURE_VALUE(32000)) {
        SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
        SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
        SETTING(voltage_out)        = 192;
        SETTING(current_gain)       = 0;        // CAM01_GAIN_140
        if (apply_dither = (SETTING(ditheringHighLight)))
            SETTING(ditheringHighLight) = false;// dither HIGH
    } else if (exposure < TO_EXPOSURE_VALUE(282000)) {
        SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
        SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
        SETTING(voltage_out)        = 224;
        SETTING(current_gain)       = 4;        // CAM01_GAIN_200
        if (apply_dither = (!SETTING(ditheringHighLight)))
            SETTING(ditheringHighLight) = true; // dither LOW
    } else if (exposure < TO_EXPOSURE_VALUE(573000)) {
        SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
        SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
        SETTING(voltage_out)        = 416;
        SETTING(current_gain)       = 8;        // CAM01_GAIN_260
        if (apply_dither = (!SETTING(ditheringHighLight)))
            SETTING(ditheringHighLight) = true; // dither LOW
    } else {
        SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
        SETTING(edge_operation)     = 3;        // CAM01_EDGEOP_NONE
        SETTING(voltage_out)        = 480;
        SETTING(current_gain)       = 12;       // CAM01_GAIN_32
        if (apply_dither = (!SETTING(ditheringHighLight)))
            SETTING(ditheringHighLight) = true; // dither LOW
    }
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_ZEROVOUT();
    if (apply_dither) RENDER_CAM_REG_DITHERPATTERN();
}


void display_last_seen(uint8_t restore) {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    uint8_t ypos = (OPTION(camera_mode) == camera_mode_manual) ? (IMAGE_DISPLAY_Y + 1) : IMAGE_DISPLAY_Y;
    screen_load_image(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, last_seen);
    if (restore) screen_restore_rect(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
}

inline void camera_scrollbars_reinit() {
    scrollbar_destroy_all();
    if (OPTION(camera_mode) == camera_mode_auto) {
        // init and set brightness scrollbar
        scrollbar_add(&ss_brightness, SS_BRIGHTNESS_X, SS_BRIGHTNESS_Y, SS_BRIGHTNESS_LEN, true);
        scrollbar_set_position(&ss_brightness, SETTING(current_brightness), 0, HISTOGRAM_MAX_VALUE);
        // init and set contrast scrollbar
        scrollbar_add(&ss_contrast, SS_CONTRAST_X, SS_CONTRAST_Y, SS_CONTRAST_LEN, false);
        scrollbar_set_position(&ss_contrast, SETTING(current_contrast), 1, NUM_CONTRAST_SETS);
    }
}

void camera_image_save() {
    static image_metadata_t image_metadata;
    uint8_t n_images = images_taken();
    if (n_images < CAMERA_MAX_IMAGE_SLOTS) {
        // modify index
        uint8_t slot = VECTOR_POP(free_slots);
        protected_modify_slot(slot, n_images);
        // copy image data
        protected_lastseen_to_slot(slot);
        // generate thumbnail
        protected_generate_thumbnail(slot);
        // save metadata
        image_metadata.raw_regs = SHADOW;
        image_metadata.settings = current_settings[OPTION(camera_mode)];
        image_metadata.crc = protected_calculate_crc((uint8_t *)&image_metadata.settings, sizeof(image_metadata.settings), PROTECTED_SEED);
        protected_metadata_write(slot, (uint8_t *)&image_metadata, sizeof(image_metadata));
        // add slot to used list
        VECTOR_ADD(used_slots, slot);
    } else music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
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
    scrollbar_repaint_all();
}

static uint8_t onPrinterProgress() BANKED {
    misc_render_progressbar(printer_completion, PRN_MAX_PROGRESS, text_buffer);
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, text_buffer);
    return 0;
}


static uint8_t vbl_frames_counter = 0;

inline void camera_charge_timer(uint8_t value) {
    COUNTER_SET(camera_shutter_timer, value);
    vbl_frames_counter = 60;
}

void shutter_VBL_ISR() NONBANKED {
    if (!vbl_frames_counter--) {
        vbl_frames_counter = 60;
        if (COUNTER(camera_shutter_timer)) {
            if (!--COUNTER(camera_shutter_timer)) camera_do_shutter = TRUE;
        }
    }
}


uint8_t INIT_state_camera() BANKED {
    CRITICAL {
        add_VBL(shutter_VBL_ISR);
    }
    return 0;
}

uint8_t ENTER_state_camera() BANKED {
#if (USE_CGB_DOUBLE_SPEED==1)
    music_setup_timer_ex(CPU_SLOW());
#endif
    // scrollbars
    camera_scrollbars_reinit();
    // repaint screen
    refresh_screen();
    // set printer progress handler
    gbprinter_set_handler(onPrinterProgress, BANK(state_camera));
    // reset capture timers and counters
    COUNTER_RESET(camera_shutter_timer);
    COUNTER_RESET(camera_repeat_counter);
    // load some initial settings
    RENDER_CAM_REGISTERS();
    SHADOW.CAM_REG_CAPTURE = 0;
    // fade in
    fade_in_modal();
    return 0;
}

// callback forward declarations
uint8_t onTranslateKeyCameraMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onIdleCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onHelpCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t * formatItemText(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings);

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

// --- Auto menu -------------------------------------
const menu_item_t CameraMenuItemAutoIndicator = {
    .prev = NULL,    .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 0, .ofs_y = 0, .width = 5, .flags = MENUITEM_TERM,
    .id = idNone,
    .caption = NULL, .helpcontext = NULL,
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};

const menu_t CameraMenuAuto = {
    .x = 0, .y = 0, .width = 0, .height = 0,
    .flags = MENU_INVERSE,
    .items = &CameraMenuItemAutoIndicator,
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
    .helpcontext = " Inverse output",
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
    return joypad_swap_dpad(value);
}
uint8_t onIdleCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu; selection;

    static const shutter_sound_t shutter_sounds[N_SHUTTER_SOUNDS] = {
        [shutter_sound_0] = {BANK(shutter01), shutter01, SFX_MUTE_MASK(shutter01)},
        [shutter_sound_1] = {BANK(shutter02), shutter02, SFX_MUTE_MASK(shutter02)}
    };

    static change_direction_e change_direction;
    static uint8_t capture_triggered = FALSE;       // state of static variable persists between calls

    // save current selection
    last_menu_items[OPTION(camera_mode)] = selection;
    // process joypad buttons
    if (KEY_PRESSED(J_A)) {
        // A is a "shutter" button
        switch (OPTION(trigger_mode)) {
            case trigger_mode_interval:
                COUNTER_SET(camera_repeat_counter, OPTION(shutter_counter));
            case trigger_mode_timer:
                camera_charge_timer(OPTION(shutter_timer));
                break;
            default:
                switch (OPTION(after_action)) {
                    case after_action_picnrec_video:
                        // toggle recording and start image capture
                        recording_video = !recording_video;
                        if (recording_video && !is_capturing()) image_capture();
                        refresh_usage_indicator();
                        break;
                    default:
                        camera_do_shutter = TRUE;
                        break;
                }
                break;
        }
    } else if (KEY_PRESSED(J_B)) {
        // cancel timers
        COUNTER_RESET(camera_shutter_timer);
        COUNTER_RESET(camera_repeat_counter);
        screen_clear_rect(18, 13, 2, 4, SOLID_BLACK);
    } else if (KEY_PRESSED(J_SELECT)) {
        // select opens popup-menu
        capture_triggered = FALSE;
        return ACTION_CAMERA_SUBMENU;
    } else if (KEY_PRESSED(J_START)) {
        // start open main menu
        capture_triggered = FALSE;
        return ACTION_MAIN_MENU;
    }

    static uint8_t selection_item_id;
    selection_item_id = selection->id;

    // !!! d-pad keys are translated
    if (OPTION(camera_mode) == camera_mode_auto) {
        // in automatic mode menu items are "synthetic"
        if (KEY_PRESSED(J_RIGHT))       change_direction = changeIncrease, selection_item_id = idBrightness;
        else if (KEY_PRESSED(J_LEFT))   change_direction = changeDecrease, selection_item_id = idBrightness;
        else if (KEY_PRESSED(J_DOWN))   change_direction = changeIncrease, selection_item_id = idContrast;
        else if (KEY_PRESSED(J_UP))     change_direction = changeDecrease, selection_item_id = idContrast;
        else change_direction = changeNone;
    } else {
        if (KEY_PRESSED(J_RIGHT))       change_direction = changeDecrease;
        else if (KEY_PRESSED(J_LEFT))   change_direction = changeIncrease;
        else change_direction = changeNone;
    }

    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    if (change_direction != changeNone) {
        static uint8_t redraw_selection;
        redraw_selection = TRUE;
        // perform changes when pressing UP/DOWN while menu item with some ID is active
        switch (selection_item_id) {
            case idExposure:
                if (redraw_selection = inc_dec_int8(&SETTING(current_exposure_idx), 1, 0, MAX_INDEX(exposures), change_direction)) {
                    SETTING(current_exposure) = exposures[SETTING(current_exposure_idx)];
                    switch (OPTION(camera_mode)) {
                        case camera_mode_assisted:
                            // ToDo: Adjust other registers ("N", Output Ref Voltage) based on index of 'current_exposure_idx'
                            RENDER_REGS_FROM_EXPOSURE();    // Voltage Out, Gain, Edge Operation, DitherLight
                            break;
                        default:
                            RENDER_CAM_REG_EXPTIME();
                            break;
                    }
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
                if (redraw_selection = inc_dec_int8(&SETTING(current_contrast), 1, 1, NUM_CONTRAST_SETS, change_direction)) {
                    RENDER_CAM_REG_DITHERPATTERN();
                    scrollbar_set_position(&ss_contrast, SETTING(current_contrast), 1, NUM_CONTRAST_SETS);
                }
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
                if (redraw_selection = inc_dec_int8(&SETTING(current_edge_ratio), 1, 0, MAX_INDEX(edge_ratios), change_direction)) RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idEdgeExclusive:
                SETTING(edge_exclusive) = !SETTING(edge_exclusive);
                RENDER_CAM_REG_EDEXOPGAIN();
                break;
            case idEdgeOperation:
                if (redraw_selection = inc_dec_int8(&SETTING(edge_operation), 1, 0, MAX_INDEX(edge_operations), change_direction)) RENDER_CAM_REG_EDEXOPGAIN();
                break;
            case idBrightness:
                if (redraw_selection = inc_dec_int16(&SETTING(current_brightness), 64, 0, HISTOGRAM_MAX_VALUE, change_direction)) {
                    scrollbar_set_position(&ss_brightness, SETTING(current_brightness), 0, HISTOGRAM_MAX_VALUE);
                }
                break;
            default:
                redraw_selection = FALSE;
                break;
        }
        // redraw selection if requested
        if (redraw_selection) {
            music_play_sfx(BANK(sound_menu_alter), sound_menu_alter, SFX_MUTE_MASK(sound_menu_alter));
            save_camera_mode_settings(OPTION(camera_mode));
            menu_move_selection(menu, NULL, selection);
        }
    }

    // process the timer
    if (COUNTER_CHANGED(camera_shutter_timer)) {
        if (camera_shutter_timer) {
            music_play_sfx(BANK(sound_timer), sound_timer, SFX_MUTE_MASK(sound_timer));
            menu_text_out(18, 15, 0, SOLID_BLACK, " " ICON_CLOCK);
            sprintf(text_buffer, " %hd", (uint8_t)COUNTER(camera_shutter_timer));
            menu_text_out(18, 16, 2, SOLID_BLACK, text_buffer);
        } else {
            screen_clear_rect(18, 15, 2, 2, SOLID_BLACK);
            if (COUNTER(camera_repeat_counter)) {
                if (--COUNTER(camera_repeat_counter)) camera_charge_timer(OPTION(shutter_timer));
            }
        }
    }

    // process the repeat counter
    if (COUNTER_CHANGED(camera_repeat_counter)) {
        if (camera_repeat_counter) {
            menu_text_out(18, 13, 0, SOLID_BLACK, " " ICON_MULTIPLE);
            sprintf(text_buffer, " %hd", (uint8_t)COUNTER(camera_repeat_counter));
            menu_text_out(18, 14, 2, SOLID_BLACK, text_buffer);
        } else screen_clear_rect(18, 13, 2, 2, SOLID_BLACK);
    }

    // make the picture if not in progress yet
    if (camera_do_shutter) {
        if (!capture_triggered) {
            music_play_sfx(shutter_sounds[OPTION(shutter_sound)].bank, shutter_sounds[OPTION(shutter_sound)].sound, shutter_sounds[OPTION(shutter_sound)].mask);
            if (!is_capturing()) image_capture();
            capture_triggered = TRUE;
        }
        camera_do_shutter = FALSE;
    }

    // check image was captured, if yes, then restart capturing process
    if (image_captured()) {
#if (ENABLE_PID==1)
        if (OPTION(camera_mode) == camera_mode_auto) {
            // P component
            int16_t error = (calculate_histogram() - SETTING(current_brightness)) / HISTOGRAM_POINTS_COUNT;
            SWITCH_RAM(CAMERA_BANK_REGISTERS);  // restore register bank after calculating

            int8_t log2_exposure = log2(SETTING(current_exposure));
            log2_exposure = MAX(log2_exposure, 1);

#define PID_P ((error >> 4) * (log2_exposure >> 2))

            // I component
#if (PID_ENABLE_I==1)
            static int16_t integral_error = 0;
            integral_error = CONSTRAINT(integral_error + error, -4096, 4096);

            static int16_t old_error = 0;
            if ((old_error ^ error) < 0) integral_error = 0; // error sign changed? reset integral component
            old_error = error;

#define PID_I ((integral_error >> 6) * (log2_exposure << 1))

#else
#define PID_I 0
#endif

            // D component
#if (PID_ENABLE_D==1)
            static int16_t old_error = 0;
            int16_t diff_error = error - old_error;
            old_error = error;

#define PID_D (diff_error >> 5)

#else
#define PID_D 0
#endif

            // apply
            SETTING(current_exposure) = CONSTRAINT(((int32_t)SETTING(current_exposure) + (PID_P + PID_I + PID_D)), CAM02_MIN_VALUE, CAM02_MAX_VALUE);
            RENDER_REGS_FROM_EXPOSURE();
            // display
            menu_text_out(15, 0, 5, SOLID_BLACK, formatItemText(idExposure, "%sms", &CURRENT_SETTINGS));
        }
#endif
        if ((recording_video) || ((capture_triggered) && (OPTION(after_action) == after_action_picnrec))) picnrec_trigger();
        display_last_seen(FALSE);
        if (capture_triggered) {
            capture_triggered = FALSE;
            switch (OPTION(after_action)) {
                case after_action_save:
                    camera_image_save();
                    refresh_usage_indicator();
                    break;
                case after_action_printsave:
                    camera_image_save();
                    refresh_usage_indicator();
                case after_action_print:
                    return ACTION_CAMERA_PRINT;
                case after_action_transfersave:
                    camera_image_save();
                    refresh_usage_indicator();
                case after_action_transfer:
                    return ACTION_CAMERA_TRANSFER;
            }
        }
        if ((image_live_preview) || (recording_video)) image_capture();
    }

    // render all present scrollbars
    hide_sprites_range(scrollbar_render_all(0), MAX_HARDWARE_SPRITES);

    // wait for VBlank if not capturing (avoid HALT CPU state)
    if (!is_capturing() && !recording_video) wait_vbl_done();
    return 0;
}
uint8_t * formatItemText(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings) {
    static const uint8_t * const on_off[]   = {"Off",    "On"} ;
    static const uint8_t * const low_high[] = {"Low",    "High"};
    static const uint8_t * const norm_inv[] = {"Normal", "Inverted"};
    switch (id) {
        case idExposure: {
            uint16_t value = FROM_EXPOSURE_VALUE(settings->current_exposure) / 100;
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
            sprintf(text_buffer, format, buf);
            break;
        }
        case idGain:
            sprintf(text_buffer, format, gains[settings->current_gain].caption);
            break;
        case idVOut:
            sprintf(text_buffer, format, settings->voltage_out);
            break;
        case idContrast:
            sprintf(text_buffer, format, settings->current_contrast);
            break;
        case idDither:
            sprintf(text_buffer, format, on_off[settings->dithering]);
            break;
        case idDitherLight:
            sprintf(text_buffer, format, low_high[settings->ditheringHighLight]);
            break;
        case idInvOutput:
            sprintf(text_buffer, format, norm_inv[settings->invertOutput]);
            break;
        case idZeroPoint:
            sprintf(text_buffer, format, zero_points[settings->current_zero_point].caption);
            break;
        case idVoltageRef:
            sprintf(text_buffer, format, voltage_refs[settings->current_voltage_ref].caption);
            break;
        case idEdgeRatio:
            sprintf(text_buffer, format, edge_ratios[settings->current_edge_ratio].caption);
            break;
        case idEdgeExclusive:
            sprintf(text_buffer, format, on_off[settings->edge_exclusive]);
            break;
        case idEdgeOperation:
            sprintf(text_buffer, format, edge_operations[settings->edge_operation].caption);
            break;
        default:
            if (format) strcpy(text_buffer, format); else *text_buffer = 0;
            break;
    }
    return text_buffer;
}
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    return formatItemText(self->id, self->caption, &CURRENT_SETTINGS);
}
uint8_t onHelpCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, selection->helpcontext);
    return 0;
}

uint8_t * camera_format_item_text(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings) BANKED {
    return formatItemText(id, format, settings);
}

uint8_t UPDATE_state_camera() BANKED {
    static uint8_t menu_result;
    JOYPAD_RESET();
    // start capturing of the image
    if ((image_live_preview) || (recording_video)) image_capture();
    // execute menu for the mode
    switch (OPTION(camera_mode)) {
        case camera_mode_manual:
            menu_result = menu_execute(&CameraMenuManual, NULL, last_menu_items[OPTION(camera_mode)]);
            break;
        case camera_mode_iterate:
        case camera_mode_assisted:
            menu_result = menu_execute(&CameraMenuAssisted, NULL, last_menu_items[OPTION(camera_mode)]);
            break;
        case camera_mode_auto:
            menu_result = menu_execute(&CameraMenuAuto, NULL, last_menu_items[OPTION(camera_mode)]);
            break;
        default:
            // error, must not get here
            menu_result = ACTION_CAMERA_SUBMENU;
            break;
    }
    switch (menu_result) {
        case ACTION_CAMERA_PRINT:
            remote_activate(REMOTE_DISABLED);
            if (gbprinter_detect(10) == PRN_STATUS_OK) {
                gbprinter_print_image(last_seen, CAMERA_BANK_LAST_SEEN, print_frames + OPTION(print_frame_idx), BANK(print_frames));
            } else music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
            remote_activate(REMOTE_ENABLED);
            break;
        case ACTION_CAMERA_TRANSFER:
            remote_activate(REMOTE_DISABLED);
            linkcable_transfer_reset();
            linkcable_transfer_image(last_seen, CAMERA_BANK_LAST_SEEN);
            remote_activate(REMOTE_ENABLED);
            break;
        case ACTION_MAIN_MENU:
            recording_video = FALSE;
            if (!menu_main_execute()) {
                COUNTER_RESET(camera_shutter_timer);
                COUNTER_RESET(camera_repeat_counter);
                refresh_screen();
            }
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
                    RENDER_CAM_REGISTERS();
                    camera_scrollbars_reinit();
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
                case ACTION_ACTION_TRANSFER:
                case ACTION_ACTION_SAVETRANSFER:
                case ACTION_ACTION_PICNREC:
                case ACTION_ACTION_PICNREC_VIDEO: {
                    static const after_action_e aactions[] = {after_action_save, after_action_print, after_action_printsave, after_action_transfer, after_action_transfersave, after_action_picnrec, after_action_picnrec_video};
                    OPTION(after_action) = aactions[menu_result - ACTION_ACTION_SAVE];
                    break;
                }
                case ACTION_RESTORE_DEFAULTS:
                    restore_default_mode_settings(OPTION(camera_mode));
                    RENDER_CAM_REGISTERS();
                    break;
                default:
                    // error, must not get here
                    music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                    break;
            }
            save_camera_state();
            COUNTER_RESET(camera_shutter_timer);
            COUNTER_RESET(camera_repeat_counter);
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
    fade_out_modal();
#if (USE_CGB_DOUBLE_SPEED==1)
    music_setup_timer_ex(CPU_FAST());
#endif
    recording_video = FALSE;
    gbprinter_set_handler(NULL, 0);
    scrollbar_destroy_all();
    return 0;
}
