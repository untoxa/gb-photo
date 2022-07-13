#pragma bank 255

#include <gbdk/platform.h>
#include <string.h>
#include <stdio.h>

#include "gbcamera.h"
#include "musicmanager.h"
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

BANKREF(state_camera)

camera_mode_e camera_mode = camera_mode_manual;
trigger_mode_e trigger_mode = trigger_mode_abutton;
after_action_e after_action = after_action_save;

uint8_t image_live_preview = TRUE;

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

void display_last_seen(uint8_t restore) {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    screen_load_image(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, last_seen);
    if (restore) screen_restore_rect(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
}

uint8_t old_capture_reg = 0;
inline uint8_t is_capturing() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    return (CAM_REG_CAPTURE & CAPTF_CAPTURING);
}
inline uint8_t image_captured() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    uint8_t v = CAM_REG_CAPTURE;
    uint8_t r = (((v ^ old_capture_reg) & CAPTF_CAPTURING) && !(v & CAPTF_CAPTURING));
    old_capture_reg = v;
    return r;
}
inline void image_capture(uint8_t capture) {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    old_capture_reg = CAM_REG_CAPTURE = capture;
}

void camera_load_settings() {
    static const uint8_t pattern[] = { 
        0x8C, 0x98, 0xAC, 0x95, 0xA7, 0xDB, 0x8E, 0x9B, 0xB7, 0x97, 0xAA, 0xE7, 0x92, 0xA2, 0xCB, 0x8F, 
        0x9D, 0xBB, 0x94, 0xA5, 0xD7, 0x91, 0xA0, 0xC7, 0x8D, 0x9A, 0xB3, 0x96, 0xA9, 0xE3, 0x8C, 0x99, 
        0xAF, 0x95, 0xA8, 0xDF, 0x93, 0xA4, 0xD3, 0x90, 0x9F, 0xC3, 0x92, 0xA3, 0xCF, 0x8F, 0x9E, 0xBF
    };
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    CAM_REG_EDEXOPGAIN  = 0xe0;
    CAM_REG_EXPTIME     = US_TO_EXPOSURE_VALUE(6000);
    CAM_REG_EDRAINVVREF = 0x03;
    CAM_REG_ZEROVOUT    = ZERO_POSITIVE | TO_VOLTAGE_OUT(192);
    memcpy(CAM_REG_DITHERPATTERN, pattern, sizeof(CAM_REG_DITHERPATTERN));
}

static void refresh_screen() {
    screen_clear_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, SOLID_BLACK);
    display_last_seen(TRUE);
}

uint8_t ENTER_state_camera() BANKED {
    refresh_screen();

    // load some initial settings
    camera_load_settings();
    if (image_live_preview) image_capture(CAPT_POSITIVE);

    return 0;     
}

uint8_t onTranslateKeyCameraMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onIdleCameraMenu(const struct menu_t * self, uint8_t * param);
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t CameraMenuItems[] = {
    {
        .prev = NULL,                   .next = CameraMenuItems + 1, 
        .sub = NULL, .sub_params = NULL,        
        .ofs_x = 0, .ofs_y = 0, .width = 5, 
        .caption = " %s",
        .onPaint = onCameraMenuItemPaint,
        .result = ACTION_SHUTTER
    }, {
        .prev = CameraMenuItems + 0,    .next = CameraMenuItems + 2, 
        .sub = NULL, .sub_params = NULL,        
        .ofs_x = 5, .ofs_y = 0, .width = 5, 
        .caption = NULL,
        .onPaint = NULL,
        .result = ACTION_SHUTTER
    }, {
        .prev = CameraMenuItems + 1,    .next = CameraMenuItems + 3, 
        .sub = NULL, .sub_params = NULL,        
        .ofs_x = 10, .ofs_y = 0, .width = 5, 
        .caption = NULL,
        .onPaint = NULL,
        .result = ACTION_SHUTTER
    }, {
        .prev = CameraMenuItems + 2,    .next = NULL, 
        .sub = NULL, .sub_params = NULL,        
        .ofs_x = 15, .ofs_y = 0, .width = 5, 
        .caption = NULL,
        .onPaint = NULL,
        .result = ACTION_SHUTTER
    }
};
const menu_t CameraMenu = {
    .x = 0, .y = 0, .width = 0, .height = 0, 
    .items = CameraMenuItems, 
    .onShow = NULL, .onIdle = onIdleCameraMenu, .onTranslateKey = onTranslateKeyCameraMenu, .onTranslateSubResult = NULL
};
uint8_t onTranslateKeyCameraMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    return (value & 0b11110000) | ((value << 1) & 0b00000100) | ((value >> 1) & 0b00000010) | ((value << 3) & 0b00001000) | ((value >> 3) & 0b00000001);
}
uint8_t onIdleCameraMenu(const struct menu_t * self, uint8_t * param) {
    self; param;
    if (image_captured()) {
        display_last_seen(TRUE);
//        if (image_live_preview) image_capture(CAPT_POSITIVE);
    } else if (KEY_PRESSED(J_SELECT)) {
        return ACTION_CAMERA_SUBMENU;
    } else wait_vbl_done();
    return 0;
}
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    if (self == (CameraMenuItems + 0)) {
        sprintf(text_buffer, self->caption, "1048ms");
    } else *text_buffer = 0;
    return text_buffer;
}

uint8_t UPDATE_state_camera() BANKED {
    static uint8_t menu_result;
    switch (menu_result = menu_execute(&CameraMenu, NULL)) {
        case ACTION_SHUTTER:
            if (!is_capturing()) {
                music_play_sfx(BANK(shutter01), shutter01, SFX_MUTE_MASK(shutter01));
//                music_play_sfx(BANK(shutter02), shutter02, SFX_MUTE_MASK(shutter02));
                image_capture(CAPT_POSITIVE);
            }
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
