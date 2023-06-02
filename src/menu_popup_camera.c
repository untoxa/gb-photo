#pragma bank 255

#include <gbdk/platform.h>
#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "globals.h"
#include "screen.h"

#include "state_camera.h"

#include "misc_assets.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_spinedit.h"

#if (PICNREC_ENABLED==1)
    #define ACTION_SUBMENU_HEIGHT 10
#else
    #define ACTION_SUBMENU_HEIGHT 8
#endif
#if (BRACKETING_ENABLED==1)
    #define MODES_SUBMENU_HEIGHT 6
#else
    #define MODES_SUBMENU_HEIGHT 5
#endif

typedef enum {
    idPopupNone = 0,
    idPopupCameraRestore,
    idPopupCameraMode,
    idPopupCameraTrigger,
    idPopupCameraAction,
    idPopupTimerValue,
    idPopupTimerCounter
} camera_popup_menu_e;


uint8_t onIdleCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onHelpCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection);
const menu_item_t ModeSubMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 9,
        .caption = " Auto",
        .helpcontext = " Full automatic mode",
        .onPaint = NULL,
        .result = ACTION_MODE_AUTO
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 9,
        .caption = " Assisted",
        .helpcontext = " Assisted mode",
        .onPaint = NULL,
        .result = ACTION_MODE_ASSISTED
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 9,
        .caption = " Manual",
        .helpcontext = " Expert mode",
        .onPaint = NULL,
        .result = ACTION_MODE_MANUAL
#if (BRACKETING_ENABLED==1)
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 9,
        .caption = " Bracketing",
        .helpcontext = " Make series of images",
        .onPaint = NULL,
        .result = ACTION_MODE_BRACKETING
#endif
    }
};
const menu_t CameraModeSubMenu = {
    .x = 5, .y = 4, .width = 11, .height = MODES_SUBMENU_HEIGHT,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_OK,
    .items = ModeSubMenuItems, .last_item = LAST_ITEM(ModeSubMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


uint8_t onTranslateSubResultTriggerSubMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
static uint8_t spinedit_timer_value;
static uint8_t spinedit_counter_value;
const spinedit_params_t TimerSpinEditParams = {
    .caption = "Timer:",
    .min_value = 1,
    .max_value = 99,
    .value = &spinedit_timer_value
};
const spinedit_value_names_t CounterSpinEditInfinite = {
    .next = NULL,
    .value = COUNTER_INFINITE_VALUE, .name = " " ICON_SPIN_UP " Inf\t" ICON_SPIN_DOWN
};
const spinedit_params_t CounterSpinEditParams = {
    .caption = "Counter:",
    .min_value = 1,
    .max_value = 31,
    .value = &spinedit_counter_value,
    .names = &CounterSpinEditInfinite
};
const menu_item_t TriggerSubMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 8,
        .caption = " " ICON_A " button",
        .helpcontext = " Trigger shutter with " ICON_A,
        .onPaint = NULL,
        .result = ACTION_TRIGGER_ABUTTON
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&TimerSpinEditParams,
        .ofs_x = 1, .ofs_y = 2, .width = 8,
        .id = idPopupTimerValue,
        .caption = " Timer",
        .helpcontext = " Use shutter timer",
        .onPaint = NULL,
        .result = ACTION_TRIGGER_TIMER
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&CounterSpinEditParams,
        .ofs_x = 1, .ofs_y = 3, .width = 8,
        .id = idPopupTimerCounter,
        .caption = " Repeat",
        .helpcontext = " Make series of pictures",
        .onPaint = NULL,
        .result = ACTION_TRIGGER_INTERVAL
    }
};
const menu_t TriggerSubMenu = {
    .x = 5, .y = 5, .width = 10, .height = 5,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_OK,
    .items = TriggerSubMenuItems, .last_item = LAST_ITEM(TriggerSubMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultTriggerSubMenu
};
uint8_t onTranslateSubResultTriggerSubMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch (self->id) {
        case idPopupTimerValue:
        case idPopupTimerCounter:
            return (value == MENU_RESULT_YES) ? self->result : MENU_RESULT_OK;
        default:
            break;
    }
    return value;
}


const menu_item_t ActionSubMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 10,
        .caption = " Save",
        .helpcontext = " Save to roll",
        .onPaint = NULL,
        .result = ACTION_ACTION_SAVE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 10,
        .caption = " Print",
        .helpcontext = " Print using GB Printer",
        .onPaint = NULL,
        .result = ACTION_ACTION_PRINT
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 10,
        .caption = " Save & Print",
        .helpcontext = " Save then Print",
        .onPaint = NULL,
        .result = ACTION_ACTION_SAVEPRINT
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 10,
        .caption = " Transfer",
        .helpcontext = " Transfer using link cable",
        .onPaint = NULL,
        .result = ACTION_ACTION_TRANSFER
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 5, .width = 10,
        .caption = " Transfer " ICON_REC,
        .helpcontext = " Transfer video by link cable",
        .onPaint = NULL,
        .result = ACTION_ACTION_TRANSF_VIDEO
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 6, .width = 10,
        .caption = " Save & Transfer",
        .helpcontext = " Save then Transfer",
        .onPaint = NULL,
        .result = ACTION_ACTION_SAVETRANSFER
#if (PICNREC_ENABLED==1)
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 7, .width = 10,
        .caption = " Pic'n'Rec",
        .helpcontext = " Save images to Pic'n'Rec",
        .onPaint = NULL,
        .result = ACTION_ACTION_PICNREC
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 8, .width = 10,
        .caption = " Pic'n'Rec " ICON_REC,
        .helpcontext = " Record video using Pic'n'Rec",
        .onPaint = NULL,
        .result = ACTION_ACTION_PICNREC_VIDEO
#endif
    }
};
const menu_t ActionSubMenu = {
    .x = 5, .y = 4, .width = 12, .height = ACTION_SUBMENU_HEIGHT,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_OK,
    .items = ActionSubMenuItems, .last_item = LAST_ITEM(ActionSubMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


uint8_t onTranslateSubResultCameraPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t * onCameraPopupMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t CameraMenuItems[] = {
    {
        .sub = &CameraModeSubMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 13,
        .id = idPopupCameraMode,
        .caption = " Mode\t\t%s",
        .helpcontext = " Select camera mode",
        .onPaint = onCameraPopupMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = &TriggerSubMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 13,
        .id = idPopupCameraTrigger,
        .caption = " Trigger\t%s",
        .helpcontext = " Trigger behavior",
        .onPaint = onCameraPopupMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = &ActionSubMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 13,
        .id = idPopupCameraAction,
        .caption = " Action\t\t%s",
        .helpcontext = " Post-processing action",
        .onPaint = onCameraPopupMenuItemPaint,
        .result = MENU_RESULT_NONE
    }, {
        .sub = &YesNoMenu, .sub_params = "Restore defaults?",
        .ofs_x = 1, .ofs_y = 4, .width = 13,
        .id = idPopupCameraRestore,
        .caption = " Restore defaults",
        .helpcontext = " Restore default settings",
        .onPaint = onCameraPopupMenuItemPaint,
        .result = ACTION_RESTORE_DEFAULTS
    }
};
const menu_t CameraPopupMenu = {
    .x = 1, .y = 3, .width = 15, .height = 6,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = CameraMenuItems, .last_item = LAST_ITEM(CameraMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultCameraPopup
};
uint8_t onTranslateSubResultCameraPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch (self->id) {
        case idPopupCameraRestore:
            return (value == MENU_RESULT_YES) ? self->result : MENU_RESULT_NO;
        default:
            break;
    }
    return value;
}
uint8_t * onCameraPopupMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    static const uint8_t * const camera_modes[N_CAMERA_MODES]  = {
        [camera_mode_manual]            = "[Manual]",
        [camera_mode_assisted]          = "[Assisted]",
        [camera_mode_auto]              = "[Auto]",
        [camera_mode_bracketing]        = "[Bracketing]"
    };
    static const uint8_t * const trigger_modes[N_TRIGGER_MODES] = {
        [trigger_mode_abutton]          = "[" ICON_A " button]",
        [trigger_mode_timer]            = "[Timer]",
        [trigger_mode_interval]         = "[Repeat]"
    };
    static const uint8_t * const after_actions[N_AFTER_ACTIONS] = {
        [after_action_save]             = "[Save]",
        [after_action_print]            = "[Print]",
        [after_action_printsave]        = "[S & P]",
        [after_action_transfer]         = "[Transfer]",
        [after_action_transfersave]     = "[S & T]",
        [after_action_picnrec]          = "[Pic'n'Rec]",
        [after_action_picnrec_video]    = "[P'n'R " ICON_REC "]",
        [after_action_transfer_video]   = "[Trn " ICON_REC"]"
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
    if (!is_capturing() && !recording_video) vsync();
    return 0;
}
uint8_t onHelpCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, selection->helpcontext);
    return 0;
}


uint8_t menu_popup_camera_execute(void) BANKED {
    spinedit_timer_value = OPTION(shutter_timer);
    spinedit_counter_value = OPTION(shutter_counter);
    uint8_t menu_result;
    switch (menu_result = menu_execute(&CameraPopupMenu, NULL, NULL)) {
        case ACTION_TRIGGER_TIMER:
            OPTION(shutter_timer) = spinedit_timer_value;
            break;
        case ACTION_TRIGGER_INTERVAL:
            OPTION(shutter_counter) = spinedit_counter_value;
            break;
        default:
            break;
    }
    return menu_result;
}

