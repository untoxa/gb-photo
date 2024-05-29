// Slitscan Settings Menu

#pragma bank 255

#include <gbdk/platform.h>
#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "globals.h"
#include "musicmanager.h"
#include "screen.h"
#include "joy.h"
#include "load_save.h"

#include "state_camera.h"
#include "mode_slitscan.h"

#include "misc_assets.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_spinedit.h"

#include "sound_menu_alter.h"

typedef enum {
    idPopupNone = 0,

    idPopupSlitscanSingleLine,
    idPopupSlitscanDelay,
    idPopupSlitscanMotionTrigger
} slitscan_popup_menu_e;


uint8_t onIdleSlitscanSubmenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t * onSlitscanSubMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onHelpSlitscanPopup(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onSlitscanPopupMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onTranslateResultSlitScanPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);

// Slitscan Delay
static uint8_t spinedit_delay_value = 0;
const spinedit_value_names_t SlitscanDelaySpinEditNames[] = {
    { .value = slitscan_delay_none,     .name = " none\t\t" },
    { .value = slitscan_delay_0_05_sec, .name = " 1/20 sec" },
    { .value = slitscan_delay_0_10_sec, .name = " 1/10 sec" },
    { .value = slitscan_delay_0_25_sec, .name = " 1/4 sec" },
    { .value = slitscan_delay_0_50_sec, .name = " 1/2 sec" },
    { .value = slitscan_delay_1_00_sec, .name = " 1 sec\t" },
};
const spinedit_params_t SlitscanDelaySpinEditParams = {
    .caption = "Delay:",
    .min_value = slitscan_delay_none,
    .max_value = MAX_INDEX(SlitscanDelaySpinEditNames),
    .value = &spinedit_delay_value,
    .names = SlitscanDelaySpinEditNames,
    .last_name = LAST_ITEM(SlitscanDelaySpinEditNames)
};
const uint8_t * const SlitscanDelayNames[] = { "none", "1/20 sec", "1/10 sec", "1/4 sec", "1/2 sec", "1 sec" };


// Slitscan MotionTrigger
static uint8_t spinedit_motiontrigger_value = 0;
const spinedit_value_names_t SlitscanMotionTriggerSpinEditNames[] = {
    { .value = slitscan_MotionTrigger_none, .name = " none" },
    { .value = slitscan_MotionTrigger_10,   .name = " 10\t" },
    { .value = slitscan_MotionTrigger_20,   .name = " 20\t" },
    { .value = slitscan_MotionTrigger_30,   .name = " 30\t" },
    { .value = slitscan_MotionTrigger_50,   .name = " 50\t" },
    { .value = slitscan_MotionTrigger_75,   .name = " 75\t" },
    { .value = slitscan_MotionTrigger_100,  .name = " 100\t" },
    { .value = slitscan_MotionTrigger_150,  .name = " 150\t" },
    { .value = slitscan_MotionTrigger_200,  .name = " 200\t" },
    { .value = slitscan_MotionTrigger_250,  .name = " 250\t" }
};
const spinedit_params_t SlitscanMotionTriggerSpinEditParams = {
    .caption = "Threshold:",
    .min_value = slitscan_MotionTrigger_none,
    .max_value = MAX_INDEX(SlitscanMotionTriggerSpinEditNames),
    .value = &spinedit_motiontrigger_value,
    .names = SlitscanMotionTriggerSpinEditNames,
    .last_name = LAST_ITEM(SlitscanMotionTriggerSpinEditNames)
};
const uint8_t * const SlitscanMotionTriggerNames[] = { "none", "10", "20", "30", "50", "75", "100", "150", "200", "250" };

// Main menu entries
const menu_item_t SlitscanMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 10,
        .id = idPopupSlitscanSingleLine,
        .caption = " Single Line\t\t%s",
        .helpcontext = " Only use a single scanline",
        .onPaint = onSlitscanSubMenuItemPaint,
        .onGetProps = onSlitscanPopupMenuItemProps,
        .result = ACTION_SLITSCAN_SINGLELINE
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&SlitscanDelaySpinEditParams,
        .ofs_x = 1, .ofs_y = 2, .width = 10,
        .id = idPopupSlitscanDelay,
        .caption = " Delay\t\t\t[%s]",
        .helpcontext = " Delay between captures\t\t",
        .onPaint = onSlitscanSubMenuItemPaint,
        .onGetProps = onSlitscanPopupMenuItemProps,
        .result = ACTION_SLITSCAN_DELAY
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&SlitscanMotionTriggerSpinEditParams,
        .ofs_x = 1, .ofs_y = 3, .width = 10,
        .id = idPopupSlitscanMotionTrigger,
        .caption = " Motion Trigger [%s]",
        .helpcontext = " Motion trigger threshold",
        .onPaint = onSlitscanSubMenuItemPaint,
        .onGetProps = onSlitscanPopupMenuItemProps,
        .result = ACTION_SLITSCAN_MOTIONTRIGGER
    }
};
// The menu itself
const menu_t SlitscanSubMenu = {
    .x = 2, .y = 4, .width = 14, .height = 5,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_OK,
    .items = SlitscanMenuItems, .last_item = LAST_ITEM(SlitscanMenuItems),
    .onShow = NULL, .onTranslateKey = NULL,
    .onTranslateSubResult = onTranslateResultSlitScanPopup,
    .onHelpContext = onHelpSlitscanPopup,
    .onIdle = onIdleSlitscanSubmenu
};

static const menu_item_t * slitscan_menu_last_selection;
uint8_t onIdleSlitscanSubmenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    slitscan_menu_last_selection = selection;
    sync_vblank();
    return 0;
}
uint8_t onHelpSlitscanPopup(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, ITEM_DEFAULT, selection->helpcontext);
    return 0;
}
uint8_t onSlitscanPopupMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    switch ((slitscan_popup_menu_e)self->id) {
        case idPopupSlitscanMotionTrigger:
            return (OPTION(slitscan_singleline) == true) ? ITEM_DEFAULT : ITEM_DISABLED;
        default:
            return ITEM_DEFAULT;
    }
}
uint8_t onTranslateResultSlitScanPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch ((slitscan_popup_menu_e)self->id) {
        case idPopupSlitscanDelay:
        case idPopupSlitscanMotionTrigger:
            return (value == MENU_RESULT_YES) ? self->result : MENU_RESULT_NO;
        default:
            break;
    }
    return value;
}




void menu_slitscan_submenu_execute(void) BANKED {
    uint8_t menu_result;
    slitscan_menu_last_selection = NULL;
    do {
        spinedit_delay_value = OPTION(slitscan_delay);
        spinedit_motiontrigger_value = OPTION(slitscan_motiontrigger);

        menu_result = menu_execute(&SlitscanSubMenu, NULL, slitscan_menu_last_selection);
        switch (menu_result) {
            case ACTION_SLITSCAN_SINGLELINE:
                OPTION(slitscan_singleline) = !OPTION(slitscan_singleline);
                save_camera_state();
                break;
            case ACTION_SLITSCAN_DELAY:
                OPTION(slitscan_delay) = spinedit_delay_value;
                save_camera_state();
                break;
            case ACTION_SLITSCAN_MOTIONTRIGGER:
                OPTION(slitscan_motiontrigger) = spinedit_motiontrigger_value;
                save_camera_state();
                break;
            case MENU_RESULT_OK:
            default:
                // unknown command or cancel
                // PLAY_SFX(sound_ok);
                break;
        }
    } while (menu_result != MENU_RESULT_OK);
}


uint8_t * onSlitscanSubMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    static const uint8_t * checkbox[] = {ICON_CBX, ICON_CBX_CHECKED};
    switch (self->id) {
        case idPopupSlitscanSingleLine:
            sprintf(text_buffer, self->caption, checkbox[OPTION(slitscan_singleline)]);
            break;
        case idPopupSlitscanDelay:
            sprintf(text_buffer, self->caption, SlitscanDelayNames[OPTION(slitscan_delay)]);
            break;
        case idPopupSlitscanMotionTrigger:
            sprintf(text_buffer, self->caption, SlitscanMotionTriggerNames[OPTION(slitscan_motiontrigger)]);
            break;
        default:
            *text_buffer = 0;
            break;
    }
    return text_buffer;
}
