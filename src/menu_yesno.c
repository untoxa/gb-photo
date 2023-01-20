#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "systemhelpers.h"
#include "screen.h"

#include "menus.h"
#include "menu_codes.h"

#include "misc_assets.h"

uint8_t onShowYesNo(const struct menu_t * self, uint8_t * param);
uint8_t onTranslateKeyYesNo(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t YesNoMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 2, .ofs_y = 3, .width = 0,
        .caption = " " ICON_B " No ",
        .onPaint = NULL,
        .result = MENU_RESULT_NO
    },{
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 7, .ofs_y = 3, .width = 0,
        .caption = " Yes",
        .onPaint = NULL,
        .result = MENU_RESULT_YES
    }
};
const menu_t YesNoMenu = {
    .x = 4, .y = 8, .width = 12, .height = 5,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = YesNoMenuItems, .last_item = LAST_ITEM(YesNoMenuItems),
    .onShow = onShowYesNo, .onTranslateKey = onTranslateKeyYesNo, .onTranslateSubResult = NULL
};

uint8_t onShowYesNo(const menu_t * self, uint8_t * param) {
    menu_draw_frame(self);
    menu_draw_shadow(self);
    menu_text_out(self->x + 1, self->y + 1, 0, BLACK_ON_WHITE, param);
    return MENU_PROP_NO_FRAME;
}

uint8_t onTranslateKeyYesNo(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    if (value & J_LEFT) value |= J_UP;
    if (value & J_RIGHT) value |= J_DOWN;
    return value;
}
