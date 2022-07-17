#include <gbdk/platform.h>
#include <stdint.h>

#include "misc_assets.h"

#include "menus.h"
#include "menu_codes.h"
#include "screen.h"

uint8_t onShowAbout(const struct menu_t * self, uint8_t * param);
const menu_item_t AboutMenuItems[] = {
    {
        .prev = NULL, .next = NULL,
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 8, .ofs_y = 4, .width = 0,
        .caption = " OK ",
        .onPaint = NULL,
        .result = MENU_RESULT_OK
    }
};
const menu_t AboutMenu = {
    .x = 4, .y = 8, .width = 12, .height = 6,
    .items = AboutMenuItems,
    .onShow = onShowAbout, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};
uint8_t onShowAbout(const menu_t * self, uint8_t * param) {
    param;
    menu_text_out(self->x + 4, self->y + 1, 0, SOLID_WHITE, "This is a");
    menu_text_out(self->x + 1, self->y + 2, 0, SOLID_WHITE, "proof-of-concept");
    return 0;
}
