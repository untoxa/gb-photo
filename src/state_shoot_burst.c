#pragma bank 255

#include <gbdk/platform.h>

#include "musicmanager.h"
#include "joy.h"
#include "screen.h"
#include "states.h"
#include "bankdata.h"

// audio assets
#include "sound_ok.h" 
#include "sound_error.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_main.h"

BANKREF(state_shoot_burst)

static void refresh_screen() {
    screen_clear_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, SOLID_BLACK);
    screen_text_out(0, 0, "\x03\xff Burst mode");
}

uint8_t ENTER_state_shoot_burst() BANKED {
    refresh_screen();
    return 0;     
}

uint8_t UPDATE_state_shoot_burst() BANKED {
    PROCESS_INPUT();
    if (KEY_PRESSED(J_START)) {
        // run Main Menu
        if (!MainMenuDispatch(menu_execute(&MainMenu, NULL))) refresh_screen();
    }
    return 0;
}

uint8_t LEAVE_state_shoot_burst() BANKED {
    return 0;     
}
