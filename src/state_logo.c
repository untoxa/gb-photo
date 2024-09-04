#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "vwf.h"
#include "musicmanager.h"
#include "joy.h"
#include "screen.h"
#include "states.h"
#include "bankdata.h"
#include "fade_manager.h"
#include "gbcamera.h"
#include "state_camera.h"

#include "misc_assets.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_main.h"

// logo
#include "logo.h"

BANKREF(state_logo)

#define Q(x) #x
#define QUOTE(x) Q(x)

static const uint8_t version_string[] = " Version " QUOTE(VERSION) " (" QUOTE(BRANCH) ")";

static void screen_load_picture(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * map, const uint8_t * tiles, uint8_t bank) {
    static uint8_t **addr, i, j;
    static const uint8_t *data;
    data = map;
    addr = (uint8_t **)(screen_tile_addresses + y);
    for (i = 0; i != h; i++, addr++) {
        for (j = 0; j != w; j++, data++) {
            uint8_t tile = read_banked_ubyte(data, bank);
            set_banked_data(*addr + ((x + j) << 4), tiles + (tile << 4), 16, bank);
        }
    }
}

static void refresh_screen(void) {
    sync_vblank();
    vwf_set_colors(DMG_WHITE, DMG_BLACK);
    screen_clear_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);
    screen_load_picture(0, 0, logo_WIDTH / logo_TILE_W, logo_HEIGHT / logo_TILE_H, logo_map, logo_tiles, BANK(logo));
    screen_text_render(0, 16, " (c) 2022..2024, Toxa");
    screen_text_render(0, 17, version_string);
    sync_vblank();
    screen_restore_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT);
}

uint8_t INIT_state_logo(void) BANKED {
    // skip logo if fast boot
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    if (OPTION(boot_to_camera_mode)) CHANGE_STATE(MAIN_STATE);

    return 0;
}

uint8_t ENTER_state_logo(void) BANKED {
    refresh_screen();
    fade_in_modal();
    JOYPAD_RESET();
    return 0;
}

uint8_t UPDATE_state_logo(void) BANKED {
    PROCESS_INPUT();
    if (KEY_PRESSED(J_START) || KEY_PRESSED(J_B)) {
        // run Main Menu
        if (!menu_main_execute()) refresh_screen();
    } else if (KEY_PRESSED(J_ANY)) {
        // any key but start enters the default state
        PLAY_SFX(sound_ok);
        CHANGE_STATE(MAIN_STATE);
        return 0;
    }
    return TRUE;
}

uint8_t LEAVE_state_logo(void) BANKED {
    fade_out_modal();
    return 0;
}
