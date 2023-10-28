#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "systemhelpers.h"
#include "globals.h"
#include "screen.h"

#include "menus.h"
#include "menu_codes.h"

#include "misc_assets.h"

uint8_t onShowMsgBox(const struct menu_t * self, uint8_t * param);
uint8_t onTranslateKeyMsgBox(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onMsgBoxItemProps(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t MsgBoxItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 5, .ofs_y = 3, .width = 4,
        .caption = ICON_A " Ok",
        .onPaint = NULL,
        .onGetProps = onMsgBoxItemProps,
        .result = MENU_RESULT_YES
    }
};
const menu_t MsgBoxMenu = {
    .x = 3, .y = 6, .width = 14, .height = 5,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = MsgBoxItems, .last_item = LAST_ITEM(MsgBoxItems),
    .onShow = onShowMsgBox, .onTranslateKey = onTranslateKeyMsgBox, .onTranslateSubResult = NULL
};
uint8_t onMsgBoxItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu; self;
    return ITEM_TEXT_CENTERED;
}

uint8_t onShowMsgBox(const menu_t * self, uint8_t * param) {
    menu_draw_frame(self);
    menu_draw_shadow(self);
    menu_text_out(self->x + 1, self->y + 1, self->width - 2, BLACK_ON_WHITE, ITEM_TEXT_CENTERED, param);
    return MENU_PROP_NO_FRAME;
}
uint8_t onTranslateKeyMsgBox(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    return value & (J_A | J_B);
}

uint8_t ShowMessageBox(const uint8_t * msg) BANKED {
    return menu_execute(&MsgBoxMenu, (uint8_t *)msg, NULL);
}

uint8_t MessageBox(const uint8_t * msg) NONBANKED {
    return ShowMessageBox(strcpy(text_buffer, msg));
}