#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "compat.h"
#include "gbcamera.h"
#include "vwf.h"
#include "screen.h"
#include "globals.h"
#include "systemhelpers.h"

#include "misc_assets.h"

#include "menus.h"
#include "menu_codes.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

static const uint8_t head_string[] = "Git: " QUOTE(BRANCH) "/" QUOTE(COMMIT);

uint8_t onShowDebug(const struct menu_t * self, uint8_t * param);
const menu_item_t DebugMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 12, .ofs_y = 13, .width = 0,
        .caption = " " ICON_A " Ok ",
        .onPaint = NULL,
        .result = MENU_RESULT_OK
    }
};
const menu_t DebugMenu = {
    .x = 3, .y = 2, .width = 17, .height = 15,
    .items = DebugMenuItems, .last_item = LAST_ITEM(DebugMenuItems),
    .onShow = onShowDebug, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

static uint8_t *render_array(uint8_t * dest, uint8_t * sour, uint8_t sour_size) {
    static const uint8_t digits[] = "0123456789abcdef";
    for (uint8_t i = 0; i != sour_size; i++) {
        *dest++ = digits[(*sour) >> 4];
        *dest++ = digits[(*sour++) & 0x0f];
        *dest++ = '\t';
    }
    *--dest = 0;
    return sour;
}

static void render_cam_image_slots(uint8_t * data_ptr, uint8_t x, uint8_t y) {
    CAMERA_SWITCH_RAM(0);

    for (uint8_t i = 0; i != 3; i++, y++) {
        data_ptr = render_array(text_buffer, data_ptr, 8);
        menu_text_out(x, y, 0, BLACK_ON_WHITE, ITEM_DEFAULT, text_buffer);
    }
    data_ptr = render_array(text_buffer, data_ptr, 6);
    menu_text_out(x, y++, 0, BLACK_ON_WHITE, ITEM_DEFAULT, text_buffer);

    memcpy(text_buffer, data_ptr, sizeof(cam_image_slots.magic)), text_buffer[sizeof(cam_image_slots.magic)] = 0;
    data_ptr += sizeof(cam_image_slots.magic);
    menu_text_out(x, y, 0, BLACK_ON_WHITE, ITEM_DEFAULT, text_buffer);

    data_ptr = render_array(text_buffer, data_ptr, 2);
    menu_text_out(x + 6, y, 0, BLACK_ON_WHITE, ITEM_DEFAULT, text_buffer);
}

uint8_t onShowDebug(const menu_t * self, uint8_t * param) {
    param;
    vwf_set_tab_size(2);
    menu_draw_frame(self);
    menu_text_out(self->x + 1, self->y + 1, 0, BLACK_ON_WHITE, ITEM_DEFAULT, head_string);
    menu_text_out(self->x + 1, self->y + 2, 0, BLACK_ON_WHITE, ITEM_DEFAULT, "Image index:");
    render_cam_image_slots((uint8_t *)&cam_image_slots, self->x + 1, self->y + 3);
    render_cam_image_slots((uint8_t *)&cam_image_slots_echo, self->x + 1, self->y + 8);
    return MENU_PROP_NO_FRAME;
}
