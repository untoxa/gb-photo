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
uint8_t onYesNoMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t YesNoMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 3, .ofs_y = 3, .width = 4,
        .caption = ICON_B " No",
        .onPaint = NULL,
        .onGetProps = onYesNoMenuItemProps,
        .result = MENU_RESULT_NO
    },{
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 8, .ofs_y = 3, .width = 4,
        .caption = "Yes",
        .onPaint = NULL,
        .onGetProps = onYesNoMenuItemProps,
        .result = MENU_RESULT_YES
    }
};
const menu_t YesNoMenu = {
    .x = 3, .y = 8, .width = 14, .height = 5,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = YesNoMenuItems, .last_item = LAST_ITEM(YesNoMenuItems),
    .onShow = onShowYesNo, .onTranslateKey = onTranslateKeyYesNo, .onTranslateSubResult = NULL
};
uint8_t onYesNoMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu; self;
    return ITEM_TEXT_CENTERED;
}

uint8_t onShowYesNo(const menu_t * self, uint8_t * param) {
    menu_draw_frame(self);
    menu_draw_shadow(self);
    menu_text_out(self->x + 1, self->y + 1, self->width - 2, BLACK_ON_WHITE, ITEM_TEXT_CENTERED, param);
    return MENU_PROP_NO_FRAME;
}
uint8_t onTranslateKeyYesNo(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    if (value & J_LEFT) value |= J_UP;
    if (value & J_RIGHT) value |= J_DOWN;
    return value;
}
