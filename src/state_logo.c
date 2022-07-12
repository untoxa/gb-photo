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

// logo
#include "logo.h"

BANKREF(state_logo)

#define Q(x) #x
#define QUOTE(x) Q(x)

static const uint8_t version_string[] = "\x03\xff Version " QUOTE(VERSION) " (" QUOTE(BRANCH) ")";

void screen_load_picture(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * map, const uint8_t * tiles, uint8_t bank) {
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

static void refresh_screen() {
    wait_vbl_done();
    screen_clear_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, SOLID_BLACK);
    screen_load_picture(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, logo_WIDTH / logo_TILE_W, logo_HEIGHT / logo_TILE_H, logo_map, logo_tiles, BANK(logo));
    screen_text_render(0, 16, "\x03\xff (c) 2022 Andreas Hahn, Toxa");
    screen_text_render(0, 17, version_string);
    wait_vbl_done();
    screen_restore_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT);
}

uint8_t ENTER_state_logo() BANKED {
    refresh_screen();
    return 0;     
}

uint8_t UPDATE_state_logo() BANKED {
    PROCESS_INPUT();
    if (KEY_PRESSED(~J_START)) {
        // any key but start enters the default state
        CHANGE_STATE(state_gallery);
        return 0;
    } else if (KEY_PRESSED(J_START)) {
        // run Main Menu
        if (!MainMenuDispatch(menu_execute(&MainMenu, NULL))) refresh_screen();
    }
    return 0;
}

uint8_t LEAVE_state_logo() BANKED {
    return 0;     
}
