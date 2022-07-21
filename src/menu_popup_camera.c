#pragma bank 255

#include <gbdk/platform.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "screen.h"

#include "state_camera.h"

#include "misc_assets.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"

#if (PICNREC_ENABLED==1)
    #define MENUITEM_PICNREC &ActionSubMenuPicNRec
    #define ACTION_SUBMENU_HEIGHT 9
#else
    #define MENUITEM_PICNREC NULL
    #define ACTION_SUBMENU_HEIGHT 7
#endif

typedef enum {
    idPopupNone = 0,
    idPopupCameraRestore,
    idPopupCameraMode,
    idPopupCameraTrigger,
    idPopupCameraAction
} camera_popup_menu_e;


uint8_t onIdleCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onHelpCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection);
const menu_item_t ModeSubMenuItemManual = {
    .prev = NULL,                       .next = &ModeSubMenuItemAssisted,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 9,
    .caption = " Manual",
    .helpcontext = " Expert mode",
    .onPaint = NULL,
    .result = ACTION_MODE_MANUAL
};
const menu_item_t ModeSubMenuItemAssisted = {
    .prev = &ModeSubMenuItemManual,     .next = &ModeSubMenuItemAuto,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 9,
    .caption = " Assisted",
    .helpcontext = " Assisted mode",
    .onPaint = NULL,
    .result = ACTION_MODE_ASSISTED
};
const menu_item_t ModeSubMenuItemAuto = {
    .prev = &ModeSubMenuItemAssisted,   .next = &ModeSubMenuItemIterate,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 9,
    .caption = " Auto",
    .helpcontext = " Full automatic mode",
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
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


const menu_item_t TriggerSubMenuItemAButton = {
    .prev = NULL,                       .next = &TriggerSubMenuItemTimer,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " " ICON_A " button",
    .helpcontext = " Trigger shutter with " ICON_A,
    .onPaint = NULL,
    .result = ACTION_TRIGGER_ABUTTON
};
const menu_item_t TriggerSubMenuItemTimer = {
    .prev = &TriggerSubMenuItemAButton, .next = &TriggerSubMenuItemInterval,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " Timer",
    .helpcontext = " Use shutter timer",
    .onPaint = NULL,
    .result = ACTION_TRIGGER_TIMER
};
const menu_item_t TriggerSubMenuItemInterval = {
    .prev = &TriggerSubMenuItemTimer,   .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 8,
    .caption = " Repeat",
    .helpcontext = " Make series of pictures",
    .onPaint = NULL,
    .result = ACTION_TRIGGER_INTERVAL
};
const menu_t TriggerSubMenu = {
    .x = 5, .y = 5, .width = 10, .height = 5,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &TriggerSubMenuItemAButton,
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


const menu_item_t ActionSubMenuSave = {
    .prev = NULL,                       .next = &ActionSubMenuPrint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 10,
    .caption = " Save",
    .helpcontext = " Save to Gallery",
    .onPaint = NULL,
    .result = ACTION_ACTION_SAVE
};
const menu_item_t ActionSubMenuPrint = {
    .prev = &ActionSubMenuSave,         .next = &ActionSubMenuSaveAndPrint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 10,
    .caption = " Print",
    .helpcontext = " Print using GB Printer",
    .onPaint = NULL,
    .result = ACTION_ACTION_PRINT
};
const menu_item_t ActionSubMenuSaveAndPrint = {
    .prev = &ActionSubMenuPrint,        .next = &ActionSubMenuTransfer,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 10,
    .caption = " Save & Print",
    .helpcontext = " Save then Print",
    .onPaint = NULL,
    .result = ACTION_ACTION_SAVEPRINT
};
const menu_item_t ActionSubMenuTransfer = {
    .prev = &ActionSubMenuSaveAndPrint, .next = &ActionSubMenuSaveAndTransfer,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 4, .width = 10,
    .caption = " Transfer",
    .helpcontext = " Transfer using link cable",
    .onPaint = NULL,
    .result = ACTION_ACTION_TRANSFER
};
const menu_item_t ActionSubMenuSaveAndTransfer = {
    .prev = &ActionSubMenuTransfer,     .next = MENUITEM_PICNREC,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 5, .width = 10,
    .caption = " Save & Transfer",
    .helpcontext = " Save then Transfer",
    .onPaint = NULL,
    .result = ACTION_ACTION_SAVETRANSFER
};
const menu_item_t ActionSubMenuPicNRec = {
    .prev = &ActionSubMenuSaveAndTransfer, .next = &ActionSubMenuPicNRecVideo,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 6, .width = 10,
    .caption = " P'n'R",
    .helpcontext = " Save images to Pic'n'Rec",
    .onPaint = NULL,
    .result = ACTION_ACTION_PICNREC
};
const menu_item_t ActionSubMenuPicNRecVideo = {
    .prev = &ActionSubMenuPicNRec,   .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 7, .width = 10,
    .caption = " P'n'R " ICON_REC,
    .helpcontext = " Record video using Pic'n'Rec",
    .onPaint = NULL,
    .result = ACTION_ACTION_PICNREC_VIDEO
};
const menu_t ActionSubMenu = {
    .x = 5, .y = 6, .width = 12, .height = ACTION_SUBMENU_HEIGHT,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &ActionSubMenuSave,
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t onTranslateSubResultCameraPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t * onCameraPopupMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);

const menu_item_t CameraMenuItemMode = {
    .prev = &CameraMenuItemReset,   .next = &CameraMenuItemTrigger,
    .sub = &CameraModeSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 13,
    .id = idPopupCameraMode,
    .caption = " Mode\t\t%s",
    .helpcontext = " Select camera mode",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemTrigger = {
    .prev = &CameraMenuItemMode,    .next = &CameraMenuItemAction,
    .sub = &TriggerSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 13,
    .id = idPopupCameraTrigger,
    .caption = " Trigger\t%s",
    .helpcontext = " Trigger behavior",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemAction = {
    .prev = &CameraMenuItemTrigger, .next = &CameraMenuItemReset,
    .sub = &ActionSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 13,
    .id = idPopupCameraAction,
    .caption = " Action\t\t%s",
    .helpcontext = " Post-processing action",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemReset = {
    .prev = &CameraMenuItemAction, .next = &CameraMenuItemMode,
    .sub = &YesNoMenu, .sub_params = "Restore defaults?",
    .ofs_x = 1, .ofs_y = 4, .width = 13, .flags = MENUITEM_TERM,
    .id = idPopupCameraRestore,
    .caption = " Restore defaults",
    .helpcontext = " Restore default settings",
    .onPaint = onCameraPopupMenuItemPaint,
    .result = ACTION_RESTORE_DEFAULTS
};

const menu_t CameraPopupMenu = {
    .x = 1, .y = 3, .width = 15, .height = 6,
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
    static const uint8_t * const camera_modes[N_CAMERA_MODES]  = {
        [camera_mode_manual]         = "[Manual]",
        [camera_mode_assisted]       = "[Assist]",
        [camera_mode_auto]           = "[Auto]",
        [camera_mode_iterate]        = "[Iter]"
    };
    static const uint8_t * const trigger_modes[N_TRIGGER_MODES] = {
        [trigger_mode_abutton]       = "[" ICON_A " button]",
        [trigger_mode_timer]         = "[Timer]",
        [trigger_mode_interval]      = "[Repeat]"
    };
    static const uint8_t * const after_actions[N_AFTER_ACTIONS] = {
        [after_action_save]          = "[Save]",
        [after_action_print]         = "[Print]",
        [after_action_printsave]     = "[S & P]",
        [after_action_transfer]      = "[Transfer]",
        [after_action_transfersave]  = "[S & T]",
        [after_action_picnrec]       = "[P'n'R]",
        [after_action_picnrec_video] = "[P'n'R " ICON_REC "]"
    };
    switch ((camera_popup_menu_e)self->id) {
        case idPopupCameraRestore:
            strcpy(text_buffer, self->caption);
            break;
        case idPopupCameraMode:
            sprintf(text_buffer, self->caption, camera_modes[OPTION(camera_mode)]);
            break;
        case idPopupCameraTrigger:
            sprintf(text_buffer, self->caption, trigger_modes[OPTION(trigger_mode)]);
            break;
        case idPopupCameraAction:
            sprintf(text_buffer, self->caption, after_actions[OPTION(after_action)]);
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
    if (!is_capturing() && !recording_video) wait_vbl_done();
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

