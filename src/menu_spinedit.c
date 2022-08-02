#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "musicmanager.h"
#include "screen.h"
#include "joy.h"
#include "globals.h"

#include "misc_assets.h"

#include "menus.h"
#include "menu_codes.h"
#include "menu_spinedit.h"

#include "sound_menu_alter.h"

spinedit_params_t * spinedit_current_params;

uint8_t onShowSpinEdit(const struct menu_t * self, uint8_t * param);
uint8_t onTranslateKeySpinEdit(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onHelpSpinEdit(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onIdleSpinEdit(const struct menu_t * menu, const struct menu_item_t * selection);
const menu_item_t SpinEditMenuItem = {
    .prev = NULL, .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 7, .ofs_y = 1, .width = 3, .flags = MENUITEM_TERM,
    .caption = " " ICON_SPIN_UP " %d\t" ICON_SPIN_DOWN,
    .helpcontext = " D-Pad Edit, " ICON_A " Ok " ICON_B " Cancel",
    .onPaint = onSpineditMenuItemPaint,
    .result = MENU_RESULT_YES
};
const menu_t SpinEditMenu = {
    .x = 4, .y = 8, .width = 14, .height = 3,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = &SpinEditMenuItem,
    .onShow = onShowSpinEdit, .onIdle = onIdleSpinEdit, .onHelpContext = onHelpSpinEdit,
    .onTranslateKey = onTranslateKeySpinEdit, .onTranslateSubResult = NULL
};
uint8_t * onSpineditMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    if (spinedit_current_params) {
        sprintf(text_buffer, self->caption, *spinedit_current_params->value);
        return text_buffer;
    }
    return 0;
}

uint8_t onShowSpinEdit(const menu_t * self, uint8_t * param) {
    spinedit_current_params = (spinedit_params_t *)param;
    menu_text_out(self->x + 1, self->y + 1, 0, SOLID_WHITE, spinedit_current_params->caption);
    return 0;
}
uint8_t onTranslateKeySpinEdit(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    // swap J_UP/J_DOWN with J_LEFT/J_RIGHT buttons, because our menus are horizontal
    return joypad_swap_dpad(value);
}
uint8_t onHelpSpinEdit(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, selection->helpcontext);
    return 0;
}

uint8_t onIdleSpinEdit(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu; selection;

    static change_direction_e change_direction;

    // !!! d-pad keys are translated
    if (KEY_PRESSED(J_RIGHT)) change_direction = changeDecrease;
    else if (KEY_PRESSED(J_LEFT)) change_direction = changeIncrease;
    else change_direction = changeNone;

    if ((change_direction != changeNone) && (spinedit_current_params)) {
        if (inc_dec_int8(spinedit_current_params->value, 1, spinedit_current_params->min_value, spinedit_current_params->max_value, change_direction)) {
            music_play_sfx(BANK(sound_menu_alter), sound_menu_alter, SFX_MUTE_MASK(sound_menu_alter));
            menu_move_selection(menu, NULL, selection);
        }
    }

    return 0;
}