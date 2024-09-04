#pragma bank 255

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat.h"
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
#include "ir.h"
#include "sd.h"

#include "globals.h"
#include "state_camera.h"
#include "state_gallery.h"
#include "pic-n-rec.h"
#include "load_save.h"

#include "misc_assets.h"

#include "cursors.h"

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
#include "menu_msgbox.h"

// dither patterns
#include "dither_patterns.h"

// frames
#include "print_frames.h"

BANKREF(state_camera)

camera_state_options_t camera_state;

bool image_live_preview = true;
bool recording_video = false;
bool camera_do_shutter = false;
bool one_iteration_autoexp = false;

COUNTER_DECLARE(camera_shutter_timer, uint8_t, 0);
COUNTER_DECLARE(camera_repeat_counter, uint8_t, 0);

COUNTER_DECLARE(camera_AEB_counter, uint8_t, 0);
uint16_t AEB_exposure_list[MAX_AEB_IMAGES];
#define last_AEB_exposure (AEB_exposure_list[MIDDLE_AEB_IMAGE])
bool AEB_capture_in_progress = false;

camera_mode_settings_t current_settings[N_CAMERA_MODES];

camera_shadow_regs_t SHADOW;        // camera shadow registers for reading

volatile uint8_t camera_PnR_delay;  // PicNRec delay counter

#define AUTOEXP_AREA_X      18
#define AUTOEXP_AREA_Y      10
#define SHUTTER_REPEAT_X    18
#define SHUTTER_REPEAT_Y    12
#define SHUTTER_TIMER_X     18
#define SHUTTER_TIMER_Y     14

#define SS_BRIGHTNESS_X     1
#define SS_BRIGHTNESS_Y     2
#define SS_BRIGHTNESS_LEN   14
static scrollbar_t ss_brightness;

#define SS_CONTRAST_X       2
#define SS_CONTRAST_Y       16
#define SS_CONTRAST_LEN     16
static scrollbar_t ss_contrast;

static const uint16_t exposures[] = {
    TO_EXPOSURE_VALUE(256),     TO_EXPOSURE_VALUE(272),     TO_EXPOSURE_VALUE(304),     TO_EXPOSURE_VALUE(352),
    TO_EXPOSURE_VALUE(400),     TO_EXPOSURE_VALUE(464),     TO_EXPOSURE_VALUE(512),     TO_EXPOSURE_VALUE(560),
    TO_EXPOSURE_VALUE(608),     TO_EXPOSURE_VALUE(704),     TO_EXPOSURE_VALUE(800),     TO_EXPOSURE_VALUE(912),
    TO_EXPOSURE_VALUE(1008),    TO_EXPOSURE_VALUE(1136),    TO_EXPOSURE_VALUE(1264),    TO_EXPOSURE_VALUE(1376),
    TO_EXPOSURE_VALUE(1504),    TO_EXPOSURE_VALUE(1744),    TO_EXPOSURE_VALUE(2000),    TO_EXPOSURE_VALUE(2256),
    TO_EXPOSURE_VALUE(2512),    TO_EXPOSURE_VALUE(2752),    TO_EXPOSURE_VALUE(3008),    TO_EXPOSURE_VALUE(3504),
    TO_EXPOSURE_VALUE(4000),    TO_EXPOSURE_VALUE(4496),    TO_EXPOSURE_VALUE(5008),    TO_EXPOSURE_VALUE(5504),
    TO_EXPOSURE_VALUE(6000),    TO_EXPOSURE_VALUE(7008),    TO_EXPOSURE_VALUE(8000),    TO_EXPOSURE_VALUE(9008),
    TO_EXPOSURE_VALUE(10000),   TO_EXPOSURE_VALUE(11264),   TO_EXPOSURE_VALUE(12512),   TO_EXPOSURE_VALUE(13760),
    TO_EXPOSURE_VALUE(15008),   TO_EXPOSURE_VALUE(17504),   TO_EXPOSURE_VALUE(20000),   TO_EXPOSURE_VALUE(22496),
    TO_EXPOSURE_VALUE(25008),   TO_EXPOSURE_VALUE(27504),   TO_EXPOSURE_VALUE(30000),   TO_EXPOSURE_VALUE(35008),
    TO_EXPOSURE_VALUE(40000),   TO_EXPOSURE_VALUE(45008),   TO_EXPOSURE_VALUE(50000),   TO_EXPOSURE_VALUE(55008),
    TO_EXPOSURE_VALUE(60000),   TO_EXPOSURE_VALUE(65008),   TO_EXPOSURE_VALUE(70000),   TO_EXPOSURE_VALUE(75008),
    TO_EXPOSURE_VALUE(80000),   TO_EXPOSURE_VALUE(90000),   TO_EXPOSURE_VALUE(100000),  TO_EXPOSURE_VALUE(112496),
    TO_EXPOSURE_VALUE(125008),  TO_EXPOSURE_VALUE(142496),  TO_EXPOSURE_VALUE(160000),  TO_EXPOSURE_VALUE(180000),
    TO_EXPOSURE_VALUE(200000),  TO_EXPOSURE_VALUE(225008),  TO_EXPOSURE_VALUE(250000),  TO_EXPOSURE_VALUE(275008),
    TO_EXPOSURE_VALUE(300000),  TO_EXPOSURE_VALUE(350000),  TO_EXPOSURE_VALUE(400000),  TO_EXPOSURE_VALUE(450000),
    TO_EXPOSURE_VALUE(500000),  TO_EXPOSURE_VALUE(550000),  TO_EXPOSURE_VALUE(600000),  TO_EXPOSURE_VALUE(700000),
    TO_EXPOSURE_VALUE(800000),  TO_EXPOSURE_VALUE(900000),  TO_EXPOSURE_VALUE(1000000), TO_EXPOSURE_VALUE(1048560)
};
static const table_value_t gains[] = {
    { CAM01_GAIN_140, "14.0" }, { CAM01_GAIN_155, "15.5" }, { CAM01_GAIN_170, "17.0" }, { CAM01_GAIN_185, "18.5" },
    { CAM01_GAIN_200, "20.0" }, { CAM01_GAIN_215, "21.5" }, { CAM01_GAIN_230, "23.0" }, { CAM01_GAIN_245, "24.5" },
    { CAM01_GAIN_260, "26.0" }, { CAM01_GAIN_275, "27.5" }, { CAM01_GAIN_290, "29.0" }, { CAM01_GAIN_305, "30.5" },
    { CAM01_GAIN_320, "32.0" }, { CAM01_GAIN_350, "35.0" }, { CAM01_GAIN_380, "38.0" }, { CAM01_GAIN_410, "41.0" },
    { CAM01_GAIN_440, "44.0" }, { CAM01_GAIN_455, "45.5" }, { CAM01_GAIN_470, "47.0" }, { CAM01_GAIN_515, "51.5" },
    { CAM01_GAIN_575, "57.5" }
};
static const table_value_t dither_patterns[N_DITHER_TYPES] = {
    [dither_type_Off]        = { dither_type_Off        , "Off"  },
    [dither_type_Default]    = { dither_type_Default    , "Def"  },
    [dither_type_2x2]        = { dither_type_2x2        , "2x2"  },
    [dither_type_Grid]       = { dither_type_Grid       , "Grid" },
    [dither_type_Maze]       = { dither_type_Maze       , "Maze" },
    [dither_type_Nest]       = { dither_type_Nest       , "Nest" },
    [dither_type_Fuzz]       = { dither_type_Fuzz       , "Fuzz" },
    [dither_type_Vertical]   = { dither_type_Vertical   , "Vert" },
    [dither_type_Horizonral] = { dither_type_Horizonral , "Hori" },
    [dither_type_Mix]        = { dither_type_Mix        , "Mix"  }
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

void RENDER_CAM_REG_EDEXOPGAIN(void)  { CAM_REG_EDEXOPGAIN  = SHADOW.CAM_REG_EDEXOPGAIN  = ((SETTING(edge_exclusive)) ? CAM01F_EDGEEXCL_V_ON : CAM01F_EDGEEXCL_V_OFF) | edge_operations[SETTING(edge_operation)].value | gains[SETTING(current_gain)].value; }
void RENDER_CAM_REG_EXPTIME(void)     { CAM_REG_EXPTIME     = SHADOW.CAM_REG_EXPTIME     = swap_bytes(SETTING(current_exposure)); }
void RENDER_CAM_REG_EDRAINVVREF(void) { CAM_REG_EDRAINVVREF = SHADOW.CAM_REG_EDRAINVVREF = edge_ratios[SETTING(current_edge_ratio)].value | ((SETTING(invertOutput)) ? CAM04F_INV : CAM04F_POS) | voltage_refs[SETTING(current_voltage_ref)].value; }
void RENDER_CAM_REG_ZEROVOUT(void)    { CAM_REG_ZEROVOUT    = SHADOW.CAM_REG_ZEROVOUT    = zero_points[SETTING(current_zero_point)].value | TO_VOLTAGE_OUT(SETTING(voltage_out)); }
inline void RENDER_CAM_REG_DITHERPATTERN(void) { dither_pattern_apply(SETTING(dithering), SETTING(ditheringHighLight), SETTING(current_contrast) - 1); }

void RENDER_CAM_REGISTERS(void) {
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_EDRAINVVREF();
    RENDER_CAM_REG_ZEROVOUT();
    RENDER_CAM_REG_DITHERPATTERN();
}

void RENDER_REGS_FROM_EXPOSURE(void) {
    // Gain 32.0 | vRef 0.480 | No edge Operation    | Exposure time range from 1048ms to 394ms
    // Gain 26.0 | vRef 0.416 | 2-D edge mode        | Exposure time range from  573ms to 164ms
    // Gain 20.0 | vRef 0.224 | 2-D edge mode        | Exposure time range from  282ms to  32ms
    // Gain 14.0 | vRef 0.192 | 2-D edge mode        | Exposure time range from   67ms to 0.8ms
    // Gain 14.0 | vRef 0.160 | Horizontal edge mode | Exposure time range from  0.5ms to 0.3ms
    bool apply_dither;
    uint16_t exposure = SETTING(current_exposure);
    if (_is_CPU_FAST) {
        if (exposure < TO_EXPOSURE_VALUE(1536)) {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
            SETTING(voltage_out)        = 160;
            SETTING(current_gain)       = 0;        // CAM01_GAIN_140
            if (apply_dither = (SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = false;// dither HIGH
        } else if (exposure < TO_EXPOSURE_VALUE(64000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = 192;
            SETTING(current_gain)       = 0;        // CAM01_GAIN_140
            if (apply_dither = (SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = false;// dither HIGH
        } else if (exposure < TO_EXPOSURE_VALUE(564000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = 224;
            SETTING(current_gain)       = 4;        // CAM01_GAIN_200
            if (apply_dither = (!SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true; // dither LOW
        } else {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = 416;
            SETTING(current_gain)       = 8;        // CAM01_GAIN_260
            if (apply_dither = (!SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true; // dither LOW
        }
    } else {
        if (exposure < TO_EXPOSURE_VALUE(768)) {
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
    }
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_ZEROVOUT();
    if (apply_dither) RENDER_CAM_REG_DITHERPATTERN();
}

void RENDER_EDGE_FROM_EXPOSURE(void) {
    uint16_t exposure = SETTING(current_exposure);
    if (_is_CPU_FAST) {
        if (exposure < TO_EXPOSURE_VALUE(1536)) {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
        } else {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
        }
    } else {
        if (exposure < TO_EXPOSURE_VALUE(768)) {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
        } else if (exposure < TO_EXPOSURE_VALUE(573000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
        } else {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 3;        // CAM01_EDGEOP_NONE
        }
    }
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
}

bool image_captured(void) {
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    if (camera_PnR_delay) return false;
    uint8_t v = CAM_REG_CAPTURE;
    bool r = (((v ^ SHADOW.CAM_REG_CAPTURE) & CAM00F_CAPTURING) && !(v & CAM00F_CAPTURING));
    SHADOW.CAM_REG_CAPTURE = v;
    return r;
}
void image_capture(void) {
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    SHADOW.CAM_REG_CAPTURE = CAM_REG_CAPTURE = (CAM00F_POSITIVE | CAM00F_CAPTURING);
    switch (OPTION(after_action)) {
        case after_action_picnrec:
        case after_action_picnrec_video:
            set_image_refresh_dalay(PNR_DELAY_FRAMES);
            break;
        default:
            break;
    }
}

void display_last_seen(bool restore) {
    CAMERA_SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    uint8_t ypos = (OPTION(camera_mode) == camera_mode_manual) ? (IMAGE_DISPLAY_Y + 1) : IMAGE_DISPLAY_Y;
    screen_load_live_image(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT,
                           last_seen,
                           OPTION(flip_live_view),
                           ((_is_COLOR) && OPTION(enable_DMA) && !((OPTION(after_action) == after_action_picnrec) || (OPTION(after_action) == after_action_picnrec_video))));
    if (restore) screen_restore_rect(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
}

inline void camera_scrollbars_reinit(void) {
    scrollbar_destroy_all();
    if (OPTION(camera_mode) == camera_mode_auto) {
        // init and set brightness scrollbar
        scrollbar_add(&ss_brightness, SS_BRIGHTNESS_X, SS_BRIGHTNESS_Y, SS_BRIGHTNESS_LEN, true);
        scrollbar_set_position(&ss_brightness, SETTING(current_brightness), 0, HISTOGRAM_MAX_VALUE);
        // init and set contrast scrollbar
        scrollbar_add(&ss_contrast, SS_CONTRAST_X, SS_CONTRAST_Y, SS_CONTRAST_LEN, false);
        scrollbar_set_position(&ss_contrast, SETTING(current_contrast), 1, NUM_CONTRAST_VALUES);
    }
}

bool camera_image_save(void) {
    static const uint8_t msgCameraRollFull[] = "Camera roll is full!";
    static image_metadata_t image_metadata;
    uint8_t n_images = images_taken();
    if (n_images < CAMERA_MAX_IMAGE_SLOTS) {
        // modify index
        uint8_t slot = VECTOR_POP(free_slots);
        protected_modify_slot(slot, n_images);
        // copy image data
        protected_lastseen_to_slot(slot, OPTION(flip_live_view));
        // generate thumbnail
        protected_generate_thumbnail(slot);
        // save metadata
        image_metadata.raw_regs = SHADOW;
        image_metadata.settings = current_settings[OPTION(camera_mode)];
        image_metadata.settings.cpu_fast = _is_CPU_FAST;
        image_metadata.crc = protected_calculate_crc((uint8_t *)&image_metadata.settings, sizeof(image_metadata.settings), PROTECTED_SEED);
        protected_metadata_write(slot, (uint8_t *)&image_metadata, sizeof(image_metadata));
        protected_image_owner_write(slot);
        // add slot to used list
        VECTOR_ADD(used_slots, slot);
        return true;
    } else {
        music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error), MUSIC_SFX_PRIORITY_HIGH);
        MessageBox(msgCameraRollFull);
        display_last_seen(true);
        return false;
    }
}

bool camera_image_save_sd(void) {
    static const uint8_t msgCameraSDError[] = "SD card error!";
    if (!lastseen_to_sd(OPTION(flip_live_view))) {
        music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error), MUSIC_SFX_PRIORITY_HIGH);
        MessageBox(msgCameraSDError);
        display_last_seen(true);
        return false;
    } else {
        return true;
    }
}

static void refresh_usage_indicator(void) {
    switch (OPTION(after_action)) {
        case after_action_picnrec_video:
        case after_action_transfer_video:
            if (recording_video) strcpy(text_buffer, ICON_REC); else *text_buffer = 0;
            break;
        default:
            sprintf(text_buffer, "%hd/%hd", (uint8_t)images_taken(), (uint8_t)images_total());
            break;
    }
    menu_text_out(HELP_CONTEXT_WIDTH, 17, IMAGE_SLOTS_USED_WIDTH, WHITE_ON_BLACK, ITEM_DEFAULT, text_buffer);
}

static void refresh_autoexp_area(void) {
    static const uint8_t * const area_indicators[N_AUTOEXP_AREAS] = {
        "", " " ICON_AUTOEXP_TOP, " " ICON_AUTOEXP_RIGHT, " " ICON_AUTOEXP_BOTTOM, " " ICON_AUTOEXP_LEFT
    };
    if (OPTION(camera_mode) != camera_mode_auto) return;
    menu_text_out(AUTOEXP_AREA_X, AUTOEXP_AREA_Y, 0, WHITE_ON_BLACK, ITEM_DEFAULT, area_indicators[OPTION(autoexp_area)]);
}

static void refresh_screen(void) {
    screen_clear_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);
    display_last_seen(true);
    refresh_usage_indicator();
    refresh_autoexp_area();
    scrollbar_repaint_all();
}

static uint8_t onPrinterProgress(void) BANKED {
    misc_render_progressbar(printer_completion, PRN_MAX_PROGRESS, text_buffer);
    menu_text_out(0, 0 + 17, HELP_CONTEXT_WIDTH, WHITE_ON_BLACK, ITEM_DEFAULT, text_buffer);
    return 0;
}

const metasprite_t grid_metasprite[] = {
    METASPR_ITEM(-4, -4, 0, 0),       METASPR_ITEM(35, 43, 0, 0), METASPR_ITEM(0, 43, 0, 0),         METASPR_ITEM(-35, 43, 0, 0),
    METASPR_ITEM(35 + 43, -43, 0, 0), METASPR_ITEM(35, 43, 0, 0), METASPR_ITEM(-35, -43 - 43, 0, 0), METASPR_ITEM(35, -43, 0, 0),
    METASPR_TERM
};
uint8_t grid_render(uint8_t hw) {
    if (OPTION(show_grid)) {
        return (hw + move_metasprite(grid_metasprite, (0x80 - cursors_TILE_COUNT), hw,
                                     DEVICE_SPRITE_PX_OFFSET_X + 16,
                                     ((OPTION(camera_mode) == camera_mode_manual) ? 8 : 0) + DEVICE_SPRITE_PX_OFFSET_Y + 16));
    }
    return 0;
}


static uint8_t vbl_frames_counter = 0;

inline void camera_charge_timer(uint8_t value) {
    COUNTER_SET(camera_shutter_timer, value);
    vbl_frames_counter = 60;
}

void shutter_VBL_ISR(void) NONBANKED {
    if (!vbl_frames_counter--) {
        vbl_frames_counter = 60;
        if (COUNTER(camera_shutter_timer)) {
            if (!--COUNTER(camera_shutter_timer)) camera_do_shutter = true;
        }
    }
    if (camera_PnR_delay) camera_PnR_delay--;
}

void reset_AEB(void) {
    if (AEB_capture_in_progress) {
        AEB_capture_in_progress = false;
        COUNTER_RESET(camera_AEB_counter);
        SETTING(current_exposure) = last_AEB_exposure;
        // if AEB capture process was cancelled, then restore exposure
        CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
        RENDER_CAM_REG_EXPTIME();
    }
}
void reset_shutter(void) {
    reset_AEB();
    // cancel timers and counters
    COUNTER_RESET(camera_shutter_timer);
    screen_clear_rect(SHUTTER_TIMER_X, SHUTTER_TIMER_Y, 2, 2, WHITE_ON_BLACK);
    COUNTER_RESET(camera_repeat_counter);
    screen_clear_rect(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y, 2, 2, WHITE_ON_BLACK);
}

uint8_t INIT_state_camera(void) BANKED {
    CRITICAL {
        add_VBL(shutter_VBL_ISR);
    }
    return 0;
}

uint8_t ENTER_state_camera(void) BANKED {
    // scrollbars
    camera_scrollbars_reinit();
    // repaint screen
    refresh_screen();
    // set printer progress handler
    gbprinter_set_handler(onPrinterProgress, BANK(state_camera));
    // On CGB, start sensing IR
    if ((_is_COLOR) && (OPTION(ir_remote_shutter))) ir_sense_start();
    // reset capture timers and counters
    COUNTER_RESET(camera_shutter_timer);
    COUNTER_RESET(camera_AEB_counter);
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
uint8_t * formatItemText(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings, bool divide_exposure);

// --- Save confirmation namu ------------------------
uint8_t onSaveConfirmMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t SaveConfirmMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 0, .ofs_y = 0, .width = 7,
        .caption = "Save image",
        .onPaint = NULL,
        .onGetProps = onSaveConfirmMenuItemProps,
        .result = MENU_RESULT_YES
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 7, .ofs_y = 0, .width = 7,
        .caption = ICON_B " Discard",
        .onPaint = NULL,
        .onGetProps = onSaveConfirmMenuItemProps,
        .result = MENU_RESULT_NO
    }
};
const menu_t SaveConfirmMenu = {
    .x = 2, .y = 17, .width = 0, .height = 0,
    .flags = MENU_FLAGS_INVERSE,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = SaveConfirmMenuItems, .last_item = LAST_ITEM(SaveConfirmMenuItems),
    .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};
uint8_t onSaveConfirmMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu; self;
    return ITEM_TEXT_CENTERED;
}

// --- Assisted menu ---------------------------------
const menu_item_t CameraMenuItemsAssisted[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 0, .ofs_y = 0, .width = 5,
        .id = idExposure,
        .caption = " %sms",
        .helpcontext = " Exposure time",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 5, .ofs_y = 0, .width = 5,
        .id = idContrast,
        .caption = " " ICON_CONTRAST "\t%d",
        .helpcontext = " Contrast level",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 10, .ofs_y = 0, .width = 5,
        .id = idDither,
        .caption = " " ICON_DITHER "\t%s",
        .helpcontext = " Dithering pattern",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        // ToDo: remove this menu option when it's being set automatically via `.id = idExposure`
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 15, .ofs_y = 0, .width = 5,
        .id = idDitherLight,
        .caption = " " ICON_DITHER "\t%s",
        .helpcontext = " Dithering light level",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }
};
const menu_t CameraMenuAssisted = {
    .x = 0, .y = 0, .width = 0, .height = 0,
    .flags = MENU_FLAGS_INVERSE,
    .items = CameraMenuItemsAssisted, .last_item = LAST_ITEM(CameraMenuItemsAssisted),
    .onShow = NULL, .onIdle = onIdleCameraMenu, .onHelpContext = onHelpCameraMenu,
    .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};

// --- Auto menu -------------------------------------
const menu_item_t CameraMenuItemsAuto[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 0, .ofs_y = 0, .width = 0,
        .id = idNone,
        .caption = " Automatic mode",
        .helpcontext = " D-Pad adjusts " ICON_BRIGHTNESS " and " ICON_CONTRAST,
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }
};
const menu_t CameraMenuAuto = {
    .x = 0, .y = 0, .width = 0, .height = 0,
    .flags = 0,
    .items = CameraMenuItemsAuto, .last_item = LAST_ITEM(CameraMenuItemsAuto),
    .onShow = NULL, .onIdle = onIdleCameraMenu, .onHelpContext = onHelpCameraMenu,
    .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};

// --- Manual menu -----------------------------------
const menu_item_t CameraMenuItemsManual[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 0, .ofs_y = 0, .width = 5,
        .id = idExposure,
        .caption = " %sms",
        .helpcontext = " Exposure time",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 5, .ofs_y = 0, .width = 5,
        .id = idGain,
        .caption = " " ICON_GAIN "\t%s",
        .helpcontext = " Sensor gain",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 10, .ofs_y = 0, .width = 5,
        .id = idDither,
        .caption = " " ICON_DITHER "\t%s",
        .helpcontext = " Dithering pattern",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 15, .ofs_y = 0, .width = 5,
        .id = idDitherLight,
        .caption = " " ICON_DITHER "\t%s",
        .helpcontext = " Dithering light level",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 0, .ofs_y = 1, .width = 5,
        .id = idContrast,
        .caption = " " ICON_CONTRAST "\t%d",
        .helpcontext = " Contrast level",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 5, .ofs_y = 1, .width = 5,
        .id = idZeroPoint,
        .caption = " %s",
        .helpcontext = " Sensor zero point",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 10, .ofs_y = 1, .width = 5,
        .id = idVOut,
        .caption = " %dmv",
        .helpcontext = " Sensor voltage out",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 15, .ofs_y = 1, .width = 5,
        .id = idVoltageRef,
        .caption = " " ICON_VOLTAGE "\t%sv",
        .helpcontext = " Sensor voltage reference",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 0, .ofs_y = 2, .width = 5,
        .id = idInvOutput,
        .caption = " %s",
        .helpcontext = " Inverse output",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 5, .ofs_y = 2, .width = 5,
        .id = idEdgeOperation,
        .caption = " " ICON_EDGE "\t%s",
        .helpcontext = " Sensor edge operation",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 10, .ofs_y = 2, .width = 5,
        .id = idEdgeRatio,
        .caption = " " ICON_EDGE "\t%s",
        .helpcontext = " Sensor edge ratio",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 15, .ofs_y = 2, .width = 5,
        .id = idEdgeExclusive,
        .caption = " " ICON_EDGE "\t%s",
        .helpcontext = "Sensor edge exclusive",
        .onPaint = onCameraMenuItemPaint,
        .result = MENU_RESULT_NONE
    }
};
const menu_t CameraMenuManual = {
    .x = 0, .y = 0, .width = 0, .height = 0,
    .flags = MENU_FLAGS_INVERSE,
    .items = CameraMenuItemsManual, .last_item = LAST_ITEM(CameraMenuItemsManual),
    .onShow = NULL, .onIdle = onIdleCameraMenu, .onHelpContext = onHelpCameraMenu,
    .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};
static const menu_item_t * last_menu_items[N_CAMERA_MODES] = { NULL, NULL, NULL };
uint8_t onTranslateKeyCameraMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    // swap J_UP/J_DOWN with J_LEFT/J_RIGHT buttons, because our menus are horizontal
    return joypad_swap_dpad(value);
}
bool isSaveCancelled(void) {
    screen_clear_rect(0, 17, HELP_CONTEXT_WIDTH, 1, WHITE_ON_BLACK);
    uint8_t menu_result = menu_execute(&SaveConfirmMenu, NULL, NULL);
    return (menu_result != MENU_RESULT_YES);
}
uint8_t onIdleCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    static const shutter_sound_t shutter_sounds[N_SHUTTER_SOUNDS] = {
        [shutter_sound_0] = {BANK(shutter01), shutter01, SFX_MUTE_MASK(shutter01)},
        [shutter_sound_1] = {BANK(shutter02), shutter02, SFX_MUTE_MASK(shutter02)}
    };
    static change_direction_e change_direction;
    static bool capture_triggered = false;       // state of static variable persists between calls
    static bool render_registers;

    // If enabled, sense remote shutters. IR sensing takes time but only if initially high
    static bool remote_shutter_triggered;
    remote_shutter_triggered = ((_is_COLOR) && OPTION(ir_remote_shutter) && !capture_triggered && ir_sense_pattern());

    // save current selection
    last_menu_items[OPTION(camera_mode)] = selection;
    // process joypad buttons
    if (KEY_PRESSED(J_START)) {
        if (OPTION(camera_mode) == camera_mode_auto) {
            PLAY_SFX(sound_menu_alter);
            // reset both brightness and contrast to defaults, adjust the sliders
            scrollbar_set_position(&ss_brightness, (SETTING(current_brightness) = HISTOGRAM_TARGET_VALUE), 0, HISTOGRAM_MAX_VALUE);
            scrollbar_set_position(&ss_contrast, (SETTING(current_contrast) = DEFAULT_CONTRAST_VALUE), 1, NUM_CONTRAST_VALUES);
            save_camera_mode_settings(OPTION(camera_mode));
            // change of contrast means reloading of the dithering pattern
            CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
            RENDER_CAM_REG_DITHERPATTERN();
        } else {
#ifdef ENABLE_AUTOEXP
            // perform one step of autoexposure if J_START is held in manual or assisted mode
            one_iteration_autoexp = true;
#endif
        }
    } else if (KEY_PRESSED(J_A) || remote_shutter_triggered) {
        // A is a "shutter" button
        switch (OPTION(after_action)) {
            case after_action_picnrec_video:
            case after_action_transfer_video:
                // toggle recording and start image capture
                recording_video = !recording_video;
                if (recording_video && !image_is_capturing()) image_capture();
                refresh_usage_indicator();
                break;
            default:
                switch (OPTION(trigger_mode)) {
                    case trigger_mode_repeat:
                        COUNTER_SET(camera_repeat_counter, OPTION(shutter_counter));
                    case trigger_mode_timer:
                        camera_charge_timer(OPTION(shutter_timer));
                        COUNTER_RESET(camera_AEB_counter);
                        break;
                    case trigger_mode_AEB: {
                            if (AEB_capture_in_progress) break;
                            AEB_capture_in_progress = true;
                            uint8_t aeb_over_counter = MIN(OPTION(aeb_overexp_count), MAX_AEB_OVEREXPOSURE);
                            uint8_t aeb_shift = (OPTION(aeb_overexp_step) & 0x01) + 3;
                            COUNTER_SET(camera_AEB_counter, (aeb_over_counter << 1) + 1);
                            // exposure mid point which is also the last_AEB_exposure
                            AEB_exposure_list[MIDDLE_AEB_IMAGE] = SETTING(current_exposure);
                            // under-exposure in i steps
                            for (uint8_t i = MIDDLE_AEB_IMAGE; i != (MIDDLE_AEB_IMAGE - aeb_over_counter); i--) {
                                AEB_exposure_list[i - 1] = CONSTRAINT((int32_t)AEB_exposure_list[i] - (AEB_exposure_list[i] >> aeb_shift), (_is_CPU_FAST) ? (EXPOSURE_LOW_LIMIT << 1) : EXPOSURE_LOW_LIMIT, EXPOSURE_HIGH_LIMIT);
                            }
                            // over-exposure in i steps
                            for (uint8_t i = MIDDLE_AEB_IMAGE; i != (MIDDLE_AEB_IMAGE + aeb_over_counter); i++) {
                                AEB_exposure_list[i + 1] = CONSTRAINT((int32_t)AEB_exposure_list[i] + (AEB_exposure_list[i] >> aeb_shift), (_is_CPU_FAST) ? (EXPOSURE_LOW_LIMIT << 1) : EXPOSURE_LOW_LIMIT, EXPOSURE_HIGH_LIMIT);
                            }
                            break;
                        }
                    default:
                        camera_do_shutter = true;
                        break;
                }
                break;
        }
    } else if (KEY_PRESSED(J_B)) {
        if (COUNTER(camera_shutter_timer) || COUNTER(camera_repeat_counter) || COUNTER(camera_AEB_counter)) {
            reset_shutter();
            camera_do_shutter = capture_triggered = false;
        } else {
            // open the main menu
            capture_triggered = false;
            return ACTION_MAIN_MENU;
        }
    } else if (KEY_PRESSED(J_SELECT)) {
        // select opens popup-menu
        capture_triggered = false;
        return ACTION_CAMERA_SUBMENU;
    }

    static uint8_t selection_item_id;
    selection_item_id = selection->id;

    // !!! d-pad keys are translated
    if (!COUNTER(camera_AEB_counter)) {
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
    } else change_direction = changeNone;                   // disable menu when capturing AEB

    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    if (change_direction != changeNone) {
        static uint8_t temp_uint8;
        static bool settings_changed, redraw_selection;
        redraw_selection = settings_changed = true;
        // perform changes when pressing UP/DOWN while menu item with some ID is active
        switch (selection_item_id) {
            case idExposure:
                if (settings_changed = inc_dec_int8(&SETTING(current_exposure_idx), (OPTION(camera_mode) == camera_mode_manual) ? 1 : 2, 0, MAX_INDEX(exposures), change_direction)) {
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
                if (settings_changed = inc_dec_int8(&SETTING(current_gain), 1, 0, MAX_INDEX(gains), change_direction)) RENDER_CAM_REG_EDEXOPGAIN();
                break;
            case idVOut:
                if (settings_changed = inc_dec_int16(&SETTING(voltage_out), VOLTAGE_OUT_STEP, MIN_VOLTAGE_OUT, MAX_VOLTAGE_OUT, change_direction)) RENDER_CAM_REG_ZEROVOUT();
                break;
            case idDither:
                temp_uint8 = SETTING(dithering);
                if (settings_changed = inc_dec_int8(&temp_uint8, 1, 0, N_DITHER_TYPES - 1, change_direction)) {
                    SETTING(dithering) = temp_uint8;
                    RENDER_CAM_REG_DITHERPATTERN();
                }
                break;
            case idDitherLight:
                SETTING(ditheringHighLight) = !SETTING(ditheringHighLight);
                RENDER_CAM_REG_DITHERPATTERN();
                break;
            case idContrast:
                if (settings_changed = inc_dec_int8(&SETTING(current_contrast), 1, 1, NUM_CONTRAST_VALUES, change_direction)) {
                    RENDER_CAM_REG_DITHERPATTERN();
                    scrollbar_set_position(&ss_contrast, SETTING(current_contrast), 1, NUM_CONTRAST_VALUES);
                    redraw_selection = (OPTION(camera_mode) != camera_mode_auto);
                }
                break;
            case idInvOutput:
                SETTING(invertOutput) = !SETTING(invertOutput);
                RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idZeroPoint:
                if (settings_changed = inc_dec_int8(&SETTING(current_zero_point), 1, 0, MAX_INDEX(zero_points), change_direction)) RENDER_CAM_REG_ZEROVOUT();
                break;
            case idVoltageRef:
                if (settings_changed = inc_dec_int8(&SETTING(current_voltage_ref), 1, 0, MAX_INDEX(voltage_refs), change_direction)) RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idEdgeRatio:
                if (settings_changed = inc_dec_int8(&SETTING(current_edge_ratio), 1, 0, MAX_INDEX(edge_ratios), change_direction)) RENDER_CAM_REG_EDRAINVVREF();
                break;
            case idEdgeExclusive:
                SETTING(edge_exclusive) = !SETTING(edge_exclusive);
                RENDER_CAM_REG_EDEXOPGAIN();
                break;
            case idEdgeOperation:
                if (settings_changed = inc_dec_int8(&SETTING(edge_operation), 1, 0, MAX_INDEX(edge_operations), change_direction)) RENDER_CAM_REG_EDEXOPGAIN();
                break;
            case idBrightness:
                if (settings_changed = inc_dec_int16(&SETTING(current_brightness), (HISTOGRAM_MAX_VALUE >> 5), 0, HISTOGRAM_MAX_VALUE, change_direction)) {
                    scrollbar_set_position(&ss_brightness, SETTING(current_brightness), 0, HISTOGRAM_MAX_VALUE);
                    redraw_selection = (OPTION(camera_mode) != camera_mode_auto);
                }
                break;
            default:
                settings_changed = false;
                break;
        }
        // redraw selection if requested
        if (settings_changed) {
            PLAY_SFX(sound_menu_alter);
            save_camera_mode_settings(OPTION(camera_mode));
            if (redraw_selection) menu_move_selection(menu, NULL, selection);
        }
    }

    // process the timer
    if (COUNTER_CHANGED(camera_shutter_timer)) {
        if (camera_shutter_timer) {
            PLAY_SFX(sound_timer);
            menu_text_out(SHUTTER_TIMER_X, SHUTTER_TIMER_Y, 0, WHITE_ON_BLACK, ITEM_DEFAULT, " " ICON_CLOCK);
            sprintf(text_buffer, " %hd", (uint8_t)COUNTER(camera_shutter_timer));
            menu_text_out(SHUTTER_TIMER_X, SHUTTER_TIMER_Y + 1, 2, WHITE_ON_BLACK, ITEM_DEFAULT, text_buffer);
        } else {
            screen_clear_rect(SHUTTER_TIMER_X, SHUTTER_TIMER_Y, 2, 2, WHITE_ON_BLACK);
            if (COUNTER(camera_repeat_counter)) {
                if (--COUNTER(camera_repeat_counter)) camera_charge_timer(OPTION(shutter_timer));
                if (OPTION(shutter_counter) == COUNTER_INFINITE_VALUE) COUNTER_SET(camera_repeat_counter, COUNTER_INFINITE_VALUE);
            }
        }
    }

    // process the repeat counter
    if (COUNTER_CHANGED(camera_repeat_counter)) {
        if (COUNTER(camera_repeat_counter)) {
            menu_text_out(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y, 0, WHITE_ON_BLACK, ITEM_DEFAULT, " " ICON_MULTIPLE);
            if (OPTION(shutter_counter) == COUNTER_INFINITE_VALUE) {
                menu_text_out(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y + 1, 2, WHITE_ON_BLACK, ITEM_DEFAULT, " Inf");
            } else {
                sprintf(text_buffer, " %hd", (uint8_t)COUNTER(camera_repeat_counter));
                menu_text_out(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y + 1, 2, WHITE_ON_BLACK, ITEM_DEFAULT, text_buffer);
            }
        } else screen_clear_rect(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y, 2, 2, WHITE_ON_BLACK);
    }

    // make the picture if not in progress yet
    if (camera_do_shutter) {
        if (!capture_triggered) {
            music_play_sfx(shutter_sounds[OPTION(shutter_sound)].bank, shutter_sounds[OPTION(shutter_sound)].sound, shutter_sounds[OPTION(shutter_sound)].mask, MUSIC_SFX_PRIORITY_NORMAL);
            if (!image_is_capturing()) image_capture();
            capture_triggered = true;
        }
        camera_do_shutter = false;
    }

    // check image was captured, if yes, then restart capturing process
    if (image_captured()) {
        switch (OPTION(after_action)) {
            case after_action_picnrec:
                if (capture_triggered) picnrec_trigger();
                break;
            case after_action_picnrec_video:
                if (recording_video) picnrec_trigger();
                break;
            case after_action_transfer_video:
                if (recording_video) {
                    remote_activate(REMOTE_DISABLED);
                    linkcable_transfer_reset();
                    linkcable_transfer_image(last_seen, CAMERA_BANK_LAST_SEEN);
                    remote_activate(REMOTE_ENABLED);
                }
                break;
            default:
                break;
        }
        display_last_seen(false);
        if (capture_triggered) {
            capture_triggered = false;
            // check save confirmation
            if (OPTION(save_confirm) && (OPTION(trigger_mode) != trigger_mode_repeat) && (OPTION(trigger_mode) != trigger_mode_AEB)) {
                if ((OPTION(after_action) == after_action_save) ||
                    (OPTION(after_action) == after_action_print) ||
                    (OPTION(after_action) == after_action_printsave) ||
                    (OPTION(after_action) == after_action_transfer) ||
                    (OPTION(after_action) == after_action_transfersave) ||
                    (OPTION(after_action) == after_action_savesd)) {
                        if (isSaveCancelled()) return ACTION_NONE;
                        onHelpCameraMenu(menu, selection);
                }
            }
            // perform after action(s)
            switch (OPTION(after_action)) {
                case after_action_save:
                    if (!camera_image_save()) {
                        reset_shutter();
                        camera_do_shutter = capture_triggered = false;
                    } else refresh_usage_indicator();
                    break;
                case after_action_printsave:
                    if (!camera_image_save()) {
                        reset_shutter();
                        camera_do_shutter = capture_triggered = false;
                    } else refresh_usage_indicator();
                case after_action_print:
                    return ACTION_CAMERA_PRINT;
                case after_action_transfersave:
                    if (!camera_image_save()) {
                        reset_shutter();
                        camera_do_shutter = capture_triggered = false;
                    } else refresh_usage_indicator();
                case after_action_transfer:
                    return ACTION_CAMERA_TRANSFER;
                case after_action_savesd:
                    if (!camera_image_save_sd()) {
                        reset_shutter();
                        camera_do_shutter = capture_triggered = false;
                    }
                    break;
                default:
                    break;
            }
        }

        // process AEB counter
        if (COUNTER_CHANGED(camera_AEB_counter)) {
            if (COUNTER(camera_AEB_counter)) {
                menu_text_out(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y, 0, WHITE_ON_BLACK, ITEM_DEFAULT, " " ICON_MULTIPLE);
                sprintf(text_buffer, " %hd", (uint8_t)COUNTER(camera_AEB_counter));
                menu_text_out(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y + 1, 2, WHITE_ON_BLACK, ITEM_DEFAULT, text_buffer);
                camera_do_shutter = true;
                // set new calculated exposure here instead of this:
                SETTING(current_exposure) = AEB_exposure_list[--COUNTER(camera_AEB_counter) + (MIDDLE_AEB_IMAGE - MIN(OPTION(aeb_overexp_count), MAX_AEB_OVEREXPOSURE))];
            } else {
                screen_clear_rect(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y, 2, 2, WHITE_ON_BLACK);
                SETTING(current_exposure) = last_AEB_exposure;
                AEB_capture_in_progress = false;
            }
            CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
            RENDER_CAM_REG_EXPTIME();
        }
#ifdef ENABLE_AUTOEXP
        else if ((one_iteration_autoexp) || (OPTION(camera_mode) == camera_mode_auto)) {
            int16_t error = (calculate_histogram(OPTION(autoexp_area)) - SETTING(current_brightness)) / HISTOGRAM_POINTS_COUNT;
            CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);  // restore register bank after histogram calculating

            int32_t new_exposure, current_exposure = SETTING(current_exposure);

            bool error_negative = (error < 0) ? true : false;
            uint16_t abs_error = abs(error);

            // real camera uses a very similar autoexposure mechanism with steps of
            // 1-1/4, 1-1/8, 1-1/16, 1-1/32, 1-1/64 on exposure time for over-exposed images
            // 1+1/8, 1+1/16, 1+1/32, 1+1/64 on exposure time for under-exposed images
            // jumps in Vref are also taken into account in real camera so that apparent exposure does not jump
            // algorithm here is globally faster and simplier than a real camera

            if (abs_error > 95) {
                // raw tuning +- 1EV
                new_exposure = (error_negative) ? (current_exposure >> 1) : (current_exposure << 1);
            } else if (abs_error > 20) {
                // intermediate tuning +- 1/8 EV
                new_exposure = current_exposure + ((error_negative) ? (0 - MAX((current_exposure >> 3), 1)) : MAX((current_exposure >> 3), 1));
            } else if (abs_error > 10) {
                // fine tuning +- 1/16 EV
                new_exposure = current_exposure + ((error_negative) ? (0 - MAX((current_exposure >> 4), 1)) : MAX((current_exposure >> 4), 1));
            } else if (abs_error > 5) {
                // very fine tuning +- 1 in C register
                new_exposure = current_exposure + ((error_negative) ? -1 : 1);
            } else new_exposure = current_exposure;

            uint16_t result_exposure = CONSTRAINT(new_exposure, (_is_CPU_FAST) ? (EXPOSURE_LOW_LIMIT << 1) : EXPOSURE_LOW_LIMIT, EXPOSURE_HIGH_LIMIT);
            if (result_exposure != SETTING(current_exposure)) {
                SETTING(current_exposure) = result_exposure;
                render_registers = true;
                if ((!one_iteration_autoexp) && (OPTION(display_exposure))) menu_text_out(14, 0, 6, WHITE_ON_BLACK, ITEM_DEFAULT, formatItemText(idExposure, "%sms", &CURRENT_SETTINGS, _is_CPU_FAST));
            } else render_registers = false;

            if ((one_iteration_autoexp) && ((JOYPAD_LAST() & J_START) == 0)) {
                one_iteration_autoexp = false;
                // restore exposure index from exposure
                for (uint8_t i = 0; i <= MAX_INDEX(exposures); i += (OPTION(camera_mode) == camera_mode_manual) ? 1 : 2) {
                    if (exposures[i] > SETTING(current_exposure)) {
                        SETTING(current_exposure_idx) = i;
                        SETTING(current_exposure) = exposures[SETTING(current_exposure_idx)];
                        render_registers = true;
                        break;
                    }
                }
                // redraw menu
                PLAY_SFX(sound_menu_alter);
                menu_redraw(menu, NULL, selection);
            }

            if (render_registers) {
                switch (OPTION(camera_mode)) {
                    case camera_mode_assisted:
                        RENDER_REGS_FROM_EXPOSURE();
                        break;
                    default:
                        RENDER_EDGE_FROM_EXPOSURE();
                        break;
                }
            }

    #if (DEBUG_AUTOEXP==1)
            sprintf(text_buffer, "%d", (uint16_t)error);
            menu_text_out(14, 1, 6, WHITE_ON_BLACK, text_buffer);
    #endif
        }
#endif
        if ((image_live_preview) || (recording_video)) image_capture();
    }

    // render grid and all present scrollbars
    hide_sprites_range(scrollbar_render_all(grid_render(0)), MAX_HARDWARE_SPRITES);

    // wait for VBlank if not capturing (avoid HALT CPU state)
    if (!image_is_capturing() && !recording_video) sync_vblank();

    return 0;
}
uint8_t * formatItemText(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings, bool divide_exposure) {
    static const uint8_t * const on_off[]   = {"Off",    "On"} ;
    static const uint8_t * const low_high[] = {"Low",    "High"};
    static const uint8_t * const norm_inv[] = {"Normal", "Inverted"};
    switch (id) {
        case idExposure: {
            uint32_t value = FROM_EXPOSURE_VALUE((divide_exposure) ? settings->current_exposure >> 1 : settings->current_exposure) / 10;
            uint8_t * buf = text_buffer_extra;
            uint8_t len = strlen(ultoa(value, buf, 10));
            // if too short value, add leading zeroes
            while (len < 3) {
                *--buf = '0', len++;
            }
            // insert comma
            len++;
            uint8_t * tail = buf + len;
            *tail-- = 0;
            *tail-- = *(tail - 1);
            *tail-- = *(tail - 1);
            *tail = ',';
            // cut trailing zeroes
            if (value < 1000) {
                tail = buf + len - 1;
                if (*tail == '0') *tail-- = 0;
                if (*tail == '0') *tail-- = 0;
                if (*tail == ',') *tail = 0;
            } else *tail = 0;
            // render
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
            sprintf(text_buffer, format, dither_patterns[settings->dithering].caption);
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
    return formatItemText(self->id, self->caption, &CURRENT_SETTINGS, _is_CPU_FAST);
}
uint8_t onHelpCameraMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, ITEM_DEFAULT, selection->helpcontext);
    return 0;
}

uint8_t * camera_format_item_text(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings) BANKED {
    return formatItemText(id, format, settings, settings->cpu_fast);
}

uint8_t UPDATE_state_camera(void) BANKED {
    static uint8_t menu_result;
    JOYPAD_RESET();
    one_iteration_autoexp = false;
    // start capturing of the image
    if ((image_live_preview) || (recording_video)) image_capture();
    // execute menu for the mode
    switch (OPTION(camera_mode)) {
        case camera_mode_manual:
            menu_result = menu_execute(&CameraMenuManual, NULL, last_menu_items[OPTION(camera_mode)]);
            break;
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
    // wait until capturing process is finished
    while (image_is_capturing());
    // process menu result
    switch (menu_result) {
        case ACTION_CAMERA_PRINT:
            remote_activate(REMOTE_DISABLED);
            if (gbprinter_detect(PRINTER_DETECT_TIMEOUT) == PRN_STATUS_OK) {
                if (gbprinter_print_image(last_seen, CAMERA_BANK_LAST_SEEN, print_frames + OPTION(print_frame_idx), BANK(print_frames)) == PRN_STATUS_CANCELLED) {
                    // cancel button pressed while printing
                    reset_AEB();
                    COUNTER_RESET(camera_shutter_timer);
                    screen_clear_rect(SHUTTER_TIMER_X, SHUTTER_TIMER_Y, 2, 2, WHITE_ON_BLACK);
                    COUNTER_RESET(camera_repeat_counter);
                    screen_clear_rect(SHUTTER_REPEAT_X, SHUTTER_REPEAT_Y, 2, 2, WHITE_ON_BLACK);
                    camera_do_shutter = false;
                };
            } else PLAY_SFX(sound_error);
            remote_activate(REMOTE_ENABLED);
            break;
        case ACTION_CAMERA_TRANSFER:
            remote_activate(REMOTE_DISABLED);
            linkcable_transfer_reset();
            linkcable_transfer_image(last_seen, CAMERA_BANK_LAST_SEEN);
            remote_activate(REMOTE_ENABLED);
            break;
        case ACTION_MAIN_MENU:
            recording_video = false;
            reset_AEB();
            if (!menu_main_execute()) {
                COUNTER_RESET(camera_shutter_timer);
                COUNTER_RESET(camera_repeat_counter);
                refresh_screen();
            }
            break;
        case ACTION_CAMERA_SUBMENU: {
            recording_video = false;
            reset_AEB();
            do {
                switch (menu_result = menu_popup_camera_execute()) {
                    case ACTION_MODE_MANUAL:
                    case ACTION_MODE_ASSISTED:
                    case ACTION_MODE_AUTO: {
                        static const camera_mode_e cmodes[] = {camera_mode_manual, camera_mode_assisted, camera_mode_auto};
                        OPTION(camera_mode) = cmodes[menu_result - ACTION_MODE_MANUAL];
                        RENDER_CAM_REGISTERS();
                        break;
                    }
                    case ACTION_TRIGGER_ABUTTON:
                    case ACTION_TRIGGER_TIMER:
                    case ACTION_TRIGGER_INTERVAL:
                    case ACTION_TRIGGER_AEB: {
                        static const trigger_mode_e tmodes[] = {trigger_mode_abutton, trigger_mode_timer, trigger_mode_repeat, trigger_mode_AEB};
                        OPTION(trigger_mode) = tmodes[menu_result - ACTION_TRIGGER_ABUTTON];
                        break;
                    }
                    case ACTION_ACTION_SAVE:
                    case ACTION_ACTION_PRINT:
                    case ACTION_ACTION_SAVEPRINT:
                    case ACTION_ACTION_TRANSFER:
                    case ACTION_ACTION_SAVETRANSFER:
                    case ACTION_ACTION_PICNREC:
                    case ACTION_ACTION_PICNREC_VIDEO:
                    case ACTION_ACTION_TRANSF_VIDEO:
                    case ACTION_ACTION_SAVESD: {
                        static const after_action_e aactions[] = {
                            after_action_save, after_action_print, after_action_printsave,
                            after_action_transfer, after_action_transfersave, after_action_picnrec,
                            after_action_picnrec_video, after_action_transfer_video, after_action_savesd
                        };
                        OPTION(after_action) = aactions[menu_result - ACTION_ACTION_SAVE];
                        break;
                    }
                    case ACTION_AUTOEXP_CENTER:
                    case ACTION_AUTOEXP_TOP:
                    case ACTION_AUTOEXP_RIGHT:
                    case ACTION_AUTOEXP_BOTTOM:
                    case ACTION_AUTOEXP_LEFT:
                        static const autoexp_area_e aareas[] = {
                            area_center, area_top, area_right, area_bottom, area_left
                        };
                        OPTION(autoexp_area) = aareas[menu_result - ACTION_AUTOEXP_CENTER];
                        break;
                    case ACTION_RESTORE_DEFAULTS:
                        restore_default_mode_settings(OPTION(camera_mode));
                    case ACTION_SET_DITHERING:
                        RENDER_CAM_REGISTERS();
                        save_camera_mode_settings(OPTION(camera_mode));
                        break;
                    default:
                        // unknown command or cancel
                        PLAY_SFX(sound_ok);
                        break;
                }
                save_camera_state();
            } while (menu_result == MENU_RESULT_NO);
            COUNTER_RESET(camera_shutter_timer);
            COUNTER_RESET(camera_repeat_counter);
            camera_scrollbars_reinit();
            refresh_screen();
            break;
        }
        default:
            // unknown command or cancel
            PLAY_SFX(sound_ok);
            break;
    }
    return FALSE;
}

uint8_t LEAVE_state_camera(void) BANKED {
    fade_out_modal();
    recording_video = false;
    reset_AEB();
    gbprinter_set_handler(NULL, 0);
    if (_is_COLOR) ir_sense_stop();
    scrollbar_destroy_all();
    return 0;
}
