#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "gbcamera.h"

#include "menus.h"
#include "menu_codes.h"
#include "screen.h"
#include "vwf.h"

#include "globals.h"

void onShowDebug(const struct menu_t * self, uint8_t * param);
const menu_item_t DebugMenuItems[] = {
    {
        .prev = NULL, .next = NULL,
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 12, .ofs_y = 13, .width = 0,
        .caption = " OK ",
        .onPaint = NULL,
        .result = MENU_RESULT_OK
    }
};
const menu_t DebugMenu = {
    .x = 3, .y = 2, .width = 17, .height = 15, 
    .items = DebugMenuItems, 
    .onShow = onShowDebug, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

static uint8_t *render_array(uint8_t * dest, uint8_t * sour, uint8_t sour_size) {
    static uint8_t digits[] = "0123456789ABCDEF";
    for (uint8_t i = 0; i != sour_size; i++) {
        *dest++ = digits[(*sour) >> 4];
        *dest++ = digits[(*sour++) & 0x0f];
        *dest++ = '\t';
    }
    *--dest = 0;
    return sour;
} 

static void render_cam_game_data(uint8_t * data_ptr, uint8_t x, uint8_t y) {
    SWITCH_RAM(0);

    for (uint8_t i = 0; i != 3; i++, y++) {
        data_ptr = render_array(text_buffer, data_ptr, 8);
        menu_text_out(x, y, 0, SOLID_WHITE, text_buffer); 
    } 
    data_ptr = render_array(text_buffer, data_ptr, 6);
    menu_text_out(x, y++, 0, SOLID_WHITE, text_buffer); 

    memcpy(text_buffer, data_ptr, sizeof(cam_game_data.magic)), text_buffer[sizeof(cam_game_data.magic)] = 0; 
    data_ptr += sizeof(cam_game_data.magic);
    menu_text_out(x, y, 0, SOLID_WHITE, text_buffer);

    data_ptr = render_array(text_buffer, data_ptr, 2);
    menu_text_out(x + 6, y, 0, SOLID_WHITE, text_buffer);
}

void onShowDebug(const menu_t * self, uint8_t * param) {
    param;
    vwf_set_tab_size(2);
    render_cam_game_data((uint8_t *)&cam_game_data, self->x + 1, self->y + 1);
    render_cam_game_data((uint8_t *)&cam_game_data_echo, self->x + 1, self->y + 7);
}
