#pragma bank 255

#include <gbdk/platform.h>

#include "gbcamera.h"
#include "musicmanager.h"
#include "joy.h"
#include "screen.h"
#include "states.h"
#include "bankdata.h"
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

static void refresh_screen() {
    screen_clear_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, SOLID_BLACK);
    display_last_seen(TRUE);
}

uint8_t ENTER_state_camera() BANKED {
    refresh_screen();
    return 0;     
}

uint8_t UPDATE_state_camera() BANKED {
    static uint8_t menu_result;

    // process capturing
    if (image_captured()) {
        display_last_seen(TRUE);
    }

    // process key input
    PROCESS_INPUT();
    if (KEY_PRESSED(J_A)) {
        if (!is_capturing()) {
            music_play_sfx(BANK(shutter01), shutter01, SFX_MUTE_MASK(shutter01));
            image_capture(CAPT_POSITIVE);
        }
    } else if (KEY_PRESSED(J_B)) {
        if (!is_capturing()) {
            music_play_sfx(BANK(shutter02), shutter02, SFX_MUTE_MASK(shutter02));
            image_capture(CAPT_NEGATIVE);
        }
    } else if (KEY_PRESSED(J_START)) {
        // run Main Menu
        if (!MainMenuDispatch(menu_execute(&MainMenu, NULL))) refresh_screen();
    } else if (KEY_PRESSED(J_SELECT)) {
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
    }
    return 0;
}

uint8_t LEAVE_state_camera() BANKED {
    return 0;     
}
