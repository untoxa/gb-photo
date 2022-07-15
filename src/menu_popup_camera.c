#pragma bank 255

#include <gbdk/platform.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "screen.h"

// camera state
#include "state_camera.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"

typedef enum {
    idPopupNone = 0,
    idPopupCameraRestore,
    idPopupCameraMode,
    idPopupCameraTrigger,
    idPopupCameraAction
} camera_popup_menu_e;


uint8_t onIdleCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection);
const menu_item_t ModeSubMenuItemManual = {
    .prev = NULL,                       .next = &ModeSubMenuItemAssisted,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 9,
    .caption = " Manual",
    .onPaint = NULL,
    .result = ACTION_MODE_MANUAL
};
const menu_item_t ModeSubMenuItemAssisted = {
    .prev = &ModeSubMenuItemManual,     .next = &ModeSubMenuItemAuto,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 9,
    .caption = " Assisted",
    .onPaint = NULL,
    .result = ACTION_MODE_ASSISTED
};
const menu_item_t ModeSubMenuItemAuto = {
    .prev = &ModeSubMenuItemAssisted,   .next = &ModeSubMenuItemIterate,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 9,
    .caption = " Auto",
    .onPaint = NULL,
    .result = ACTION_MODE_AUTO
};
const menu_item_t ModeSubMenuItemIterate = {
    .prev = &ModeSubMenuItemAuto,       .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 4, .width = 9,
    .caption = " Iterate",
    .onPaint = NULL,
    .result = ACTION_MODE_ITERATE
};
const menu_t CameraModeSubMenu = {
    .x = 5, .y = 4, .width = 11, .height = 6,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &ModeSubMenuItemManual,
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


const menu_item_t TriggerSubMenuItemAButton = {
    .prev = NULL,                       .next = &TriggerSubMenuItemTimer,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " A Button",
    .onPaint = NULL,
    .result = ACTION_TRIGGER_ABUTTON
};
const menu_item_t TriggerSubMenuItemTimer = {
    .prev = &TriggerSubMenuItemAButton, .next = &TriggerSubMenuItemInterval,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " Timer",
    .onPaint = NULL,
    .result = ACTION_TRIGGER_TIMER
};
const menu_item_t TriggerSubMenuItemInterval = {
    .prev = &TriggerSubMenuItemTimer,   .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 8,
    .caption = " Repeat",
    .onPaint = NULL,
    .result = ACTION_TRIGGER_INTERVAL
};
const menu_t TriggerSubMenu = {
    .x = 5, .y = 5, .width = 10, .height = 5,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &TriggerSubMenuItemAButton,
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


const menu_item_t ActionSubMenuSave = {
    .prev = NULL,                       .next = &ActionSubMenuPrint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " Save",
    .onPaint = NULL,
    .result = ACTION_ACTION_SAVE
};
const menu_item_t ActionSubMenuPrint = {
    .prev = &ActionSubMenuSave, .next = &ActionSubMenuSaveAndPrint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " Print",
    .onPaint = NULL,
    .result = ACTION_ACTION_PRINT
};
const menu_item_t ActionSubMenuSaveAndPrint = {
    .prev = &ActionSubMenuPrint,   .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 8,
    .caption = " Save & Print",
    .onPaint = NULL,
    .result = ACTION_ACTION_SAVEPRINT
};
const menu_t ActionSubMenu = {
    .x = 5, .y = 6, .width = 10, .height = 5,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &ActionSubMenuSave,
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t onTranslateSubResultCameraPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t * onCameraPopupMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onHelpCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection);

const menu_item_t CameraMenuItemMode = {
    .prev = &CameraMenuItemReset,   .next = &CameraMenuItemTrigger,
    .sub = &CameraModeSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 11,
    .id = idPopupCameraMode,
    .helpcontext = " Select camera mode",
    .caption = " Mode\t\t%s",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemTrigger = {
    .prev = &CameraMenuItemMode,    .next = &CameraMenuItemAction,
    .sub = &TriggerSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 11,
    .id = idPopupCameraTrigger,
    .helpcontext = " Trigger behavior",
    .caption = " Trigger\t%s",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemAction = {
    .prev = &CameraMenuItemTrigger, .next = &CameraMenuItemReset,
    .sub = &ActionSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 11,
    .id = idPopupCameraAction,
    .helpcontext = " Post-processing action",
    .caption = " Action\t\t%s",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemReset = {
    .prev = &CameraMenuItemAction, .next = &CameraMenuItemMode,
    .sub = &YesNoMenu, .sub_params = "Restore defaults?",
    .ofs_x = 1, .ofs_y = 4, .width = 11, .flags = MENUITEM_TERM,
    .id = idPopupCameraRestore,
    .caption = " Restore defaults",
    .helpcontext = " Restore default settings",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = ACTION_RESTORE_DEFAULTS
};

const menu_t CameraPopupMenu = {
    .x = 1, .y = 3, .width = 13, .height = 6,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &CameraMenuItemMode,
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultCameraPopup
};
uint8_t onTranslateSubResultCameraPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    if (self->id == idPopupCameraRestore) {
        return (value == MENU_RESULT_YES) ? self->result : ACTION_NONE;
    }
    return value;
}
uint8_t * onCameraPopupMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    static const uint8_t * const camera_modes[]  = {"[Manual]", "[Assist]", "[Auto]", "[Iter]"};
    static const uint8_t * const trigger_modes[] = {"[Btn: A]", "[Timer]", "[Repeat]"};
    static const uint8_t * const after_actions[] = {"[Save]", "[Print]", "[S & P]"};
    switch ((camera_popup_menu_e)self->id) {
        case idPopupCameraRestore:
            strcpy(text_buffer, self->caption);
            break;
        case idPopupCameraMode:
            sprintf(text_buffer, self->caption, camera_modes[camera_mode]);
            break;
        case idPopupCameraTrigger:
            sprintf(text_buffer, self->caption, trigger_modes[trigger_mode]);
            break;
        case idPopupCameraAction:
            sprintf(text_buffer, self->caption, after_actions[after_action]);
            break;
        default:
            *text_buffer = 0;
            break;
    }
    return text_buffer;
}
uint8_t onIdleCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu; selection;
    // wait for VBlank if not capturing (avoid HALT CPU state)
    if (!is_capturing()) wait_vbl_done();
    return 0;
}
uint8_t onHelpCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, selection->helpcontext);
    return 0;
}

uint8_t menu_popup_camera_execute() BANKED {
    return menu_execute(&CameraPopupMenu, NULL, NULL);
}

