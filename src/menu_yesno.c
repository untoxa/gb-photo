#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "menus.h"
#include "menu_codes.h"
#include "screen.h"

void onShowYesNo(const struct menu_t * self, uint8_t * param);
uint8_t onTranslateKeyYesNo(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t YesNoMenuItems[] = {
    {
        .prev = NULL, .next = &YesNoMenuItems[1],
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 2, .ofs_y = 3, .width = 0,
        .caption = " NO ",
        .onPaint = NULL,
        .result = MENU_RESULT_NO
    },{
        .prev = &YesNoMenuItems[0], .next = NULL,
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 7, .ofs_y = 3, .width = 0,
        .caption = " YES",
        .onPaint = NULL,
        .result = MENU_RESULT_YES
    }
};
const menu_t YesNoMenu = {
    .x = 4, .y = 8, .width = 12, .height = 5, 
    .items = YesNoMenuItems, 
    .onShow = onShowYesNo, .onTranslateKey = onTranslateKeyYesNo, .onTranslateSubResult = NULL
};

void onShowYesNo(const menu_t * self, uint8_t * param) {
    menu_text_out(self->x + 1, self->y + 1, 0, SOLID_WHITE, param); 
}

uint8_t onTranslateKeyYesNo(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    if (value & J_LEFT) value |= J_UP;
    if (value & J_RIGHT) value |= J_DOWN;
    return value;
}
