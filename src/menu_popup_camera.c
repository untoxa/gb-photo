#pragma bank 255

#include <gbdk/platform.h>
#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "globals.h"
#include "musicmanager.h"
#include "screen.h"
#include "joy.h"

#include "state_camera.h"
#include "dither_patterns.h"
#include "userinfo.h"

#include "misc_assets.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_yesno.h"
#include "menu_spinedit.h"
#include "menu_edit.h"

#include "sound_menu_alter.h"

typedef enum {
    idPopupNone = 0,
    idPopupCameraRestore,
    idPopupCameraMode,
    idPopupCameraTrigger,
    idPopupCameraAction,
    idPopupCameraArea,
    idPopupTriggerAButton,
    idPopupTriggerTimerValue,
    idPopupTriggerTimerCounter,
    idPopupTriggerAEB,
    idPopupCameraDither,
    idPopupCameraOwnerOk,
    idPopupCameraOwnerInfo,
    idPopupCameraOwnerName,
    idPopupCameraOwnerGender,
    idPopupCameraOwnerBirth,
    idPopupActionSaveSD,
    idPopupActionPicNRec,
    idPopupActionPrintPicNRec,
    idPopupActionPicNRecVideo
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
    }
};
const menu_t CameraModeSubMenu = {
    .x = 5, .y = 4, .width = 11, .height = LENGTH(ModeSubMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = ModeSubMenuItems, .last_item = LAST_ITEM(ModeSubMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t * onAEBMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onShowAEBMenu(const menu_t * self, uint8_t * param);
uint8_t onTranslateKeyAEBMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onIdleAEBMenu(const struct menu_t * menu, const struct menu_item_t * selection);
static uint8_t spinedit_aeb_overexp_count;
static uint8_t spinedit_aeb_exp_step;
const spinedit_params_t AEBOverexpSpinEditParams = {
    .caption = "Steps:",
    .min_value = 2,
    .max_value = MAX_AEB_OVEREXPOSURE,
    .value = &spinedit_aeb_overexp_count
};
const spinedit_value_names_t AEBStepNames[] = {
    { .value = 0, .name = " " ICON_SPIN_UP " 2 EV\t" ICON_SPIN_DOWN },
    { .value = 1, .name = " " ICON_SPIN_UP " 1 EV\t" ICON_SPIN_DOWN}
};
const spinedit_params_t AEBStepSpinEditParams = {
    .caption = "Range:",
    .min_value = 0,
    .max_value = MAX_INDEX(AEBStepNames),
    .value = &spinedit_aeb_exp_step,
    .names = AEBStepNames,
    .last_name = LAST_ITEM(AEBStepNames)
};
const menu_item_t AEBMenuItems[] = {
    {
        .sub = NULL, .sub_params = (uint8_t *)&AEBOverexpSpinEditParams,
        .ofs_x = 6, .ofs_y = 1, .width = 3,
        .caption = " " ICON_SPIN_UP " %d\t\t" ICON_SPIN_DOWN,
        .helpcontext = " D-Pad Edit, " ICON_A " Ok " ICON_B " Cancel",
        .onPaint = onAEBMenuItemPaint,
        .result = MENU_RESULT_YES
    }, {
        .sub = NULL, .sub_params = (uint8_t *)&AEBStepSpinEditParams,
        .ofs_x = 6, .ofs_y = 2, .width = 3,
        .caption = " " ICON_SPIN_UP " %d\t\t" ICON_SPIN_DOWN,
        .helpcontext = " D-Pad Edit, " ICON_A " Ok " ICON_B " Cancel",
        .onPaint = onAEBMenuItemPaint,
        .result = MENU_RESULT_YES
    }
};
const menu_t AEBMenu = {
    .x = 4, .y = 8, .width = 14, .height = 4,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = AEBMenuItems, .last_item = LAST_ITEM(AEBMenuItems),
    .onShow = onShowAEBMenu, .onIdle = onIdleAEBMenu, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = onTranslateKeyAEBMenu, .onTranslateSubResult = NULL
};
uint8_t * onAEBMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    if (self->sub_params) {
        const spinedit_params_t * params = (const spinedit_params_t *)self->sub_params;
        uint8_t value = *params->value;
        const spinedit_value_names_t * current_name = params->names;
        while (current_name <= params->last_name) {
            if (current_name->value == value) return strcpy(text_buffer, current_name->name);
            current_name++;
        }
        sprintf(text_buffer, self->caption, value);
        return text_buffer;
    }
    return 0;
}
uint8_t onShowAEBMenu(const menu_t * self, uint8_t * param) {
    param;
    menu_draw_frame(self);
    menu_draw_shadow(self);
    menu_text_out(self->x + 1, self->y + 1, 0, BLACK_ON_WHITE, ITEM_DEFAULT, AEBOverexpSpinEditParams.caption);
    menu_text_out(self->x + 1, self->y + 2, 0, BLACK_ON_WHITE, ITEM_DEFAULT, AEBStepSpinEditParams.caption);
    return MENU_PROP_NO_FRAME;
}
uint8_t onTranslateKeyAEBMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;
    // swap J_UP/J_DOWN with J_LEFT/J_RIGHT buttons, because our menus are horizontal
    return joypad_swap_dpad(value);
}
uint8_t onIdleAEBMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu; selection;
    static change_direction_e change_direction;

    // !!! d-pad keys are translated
    if (KEY_PRESSED(J_RIGHT)) change_direction = changeDecrease;
    else if (KEY_PRESSED(J_LEFT)) change_direction = changeIncrease;
    else change_direction = changeNone;

    if ((change_direction != changeNone) && (selection->sub_params)) {
        spinedit_params_t * params = (spinedit_params_t *)selection->sub_params;
        if (inc_dec_int8(params->value, 1, params->min_value, params->max_value, change_direction)) {
            PLAY_SFX(sound_menu_alter);
            menu_move_selection(menu, NULL, selection);
            if (params->onChange) params->onChange(menu, selection, params);
        }
    }

    sync_vblank();
    return 0;
}

uint8_t onTranslateSubResultTriggerSubMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
static uint8_t spinedit_timer_value;
static uint8_t spinedit_counter_value;
const spinedit_params_t TimerSpinEditParams = {
    .caption = "Timer:",
    .min_value = 1,
    .max_value = 99,
    .value = &spinedit_timer_value
};
const spinedit_value_names_t CounterSpinEditInfinite[] = {
    { .value = COUNTER_INFINITE_VALUE, .name = " " ICON_SPIN_UP " Inf\t" ICON_SPIN_DOWN }
};
const spinedit_params_t CounterSpinEditParams = {
    .caption = "Counter:",
    .min_value = 1,
    .max_value = 31,
    .value = &spinedit_counter_value,
    .names = CounterSpinEditInfinite,
    .last_name = LAST_ITEM(CounterSpinEditInfinite)
};
const menu_item_t TriggerSubMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 8,
        .id = idPopupTriggerAButton,
        .caption = " " ICON_A " button",
        .helpcontext = " Trigger shutter with " ICON_A,
        .onPaint = NULL,
        .result = ACTION_TRIGGER_ABUTTON
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&TimerSpinEditParams,
        .ofs_x = 1, .ofs_y = 2, .width = 8,
        .id = idPopupTriggerTimerValue,
        .caption = " Timer",
        .helpcontext = " Use shutter timer",
        .onPaint = NULL,
        .result = ACTION_TRIGGER_TIMER
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&CounterSpinEditParams,
        .ofs_x = 1, .ofs_y = 3, .width = 8,
        .id = idPopupTriggerTimerCounter,
        .caption = " Repeat",
        .helpcontext = " Make series of pictures",
        .onPaint = NULL,
        .result = ACTION_TRIGGER_INTERVAL
#if (AEB_ENABLED==1)
    }, {
        .sub = &AEBMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 8,
        .id = idPopupTriggerAEB,
        .caption = " AEB mode",
        .helpcontext = " Auto Exposure Bracketing",
        .onPaint = NULL,
        .result = ACTION_TRIGGER_AEB
#endif
    }
};
const menu_t TriggerSubMenu = {
    .x = 5, .y = 5, .width = 10, .height = LENGTH(TriggerSubMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = TriggerSubMenuItems, .last_item = LAST_ITEM(TriggerSubMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultTriggerSubMenu
};
uint8_t onTranslateSubResultTriggerSubMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch (self->id) {
        case idPopupTriggerTimerValue:
        case idPopupTriggerTimerCounter:
        case idPopupTriggerAEB:
            return (value == MENU_RESULT_YES) ? self->result : MENU_RESULT_NO;
        default:
            break;
    }
    return value;
}


uint8_t onActionsMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
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
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 7, .width = 10,
        .id =  idPopupActionSaveSD,
        .caption = " Save to SD",
        .helpcontext = " Save to SD card",
        .onPaint = NULL,
        .onGetProps = onActionsMenuItemProps,
        .result = ACTION_ACTION_SAVESD
#if (PICNREC_ENABLED==1)
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 8, .width = 10,
        .id = idPopupActionPicNRec,
        .caption = " Pic'n'Rec",
        .helpcontext = " Save images to Pic'n'Rec",
        .onPaint = NULL,
        .onGetProps = onActionsMenuItemProps,
        .result = ACTION_ACTION_PICNREC
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 9, .width = 10,
        .id = idPopupActionPrintPicNRec,
        .caption = " Print & P'n'R",
        .helpcontext = " Print & Save to Pic'n'Rec",
        .onPaint = NULL,
        .onGetProps = onActionsMenuItemProps,
        .result = ACTION_ACTION_PRINTPICNREC
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 10, .width = 10,
        .id = idPopupActionPicNRecVideo,
        .caption = " Pic'n'Rec " ICON_REC,
        .helpcontext = " Record video using Pic'n'Rec",
        .onPaint = NULL,
        .onGetProps = onActionsMenuItemProps,
        .result = ACTION_ACTION_PICNREC_VIDEO
#endif
    }
};
const menu_t ActionSubMenu = {
    .x = 5, .y = 2, .width = 12, .height = LENGTH(ActionSubMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = ActionSubMenuItems, .last_item = LAST_ITEM(ActionSubMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};
uint8_t onActionsMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu; self;
    switch (self->id) {
        case idPopupActionPicNRec:
        case idPopupActionPrintPicNRec:
        case idPopupActionPicNRecVideo:
            return (OPTION(cart_type) == cart_type_iG_AIO) ? ITEM_DEFAULT : ITEM_DISABLED;
        case idPopupActionSaveSD:
            #if (SD_ENABLED==1)
            return ITEM_DEFAULT;
            #else
            return ITEM_DISABLED;
            #endif
        default:
            return ITEM_DEFAULT;
    }
}

const menu_item_t AutoexpAreaSubMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 9,
        .caption = " Center",
        .helpcontext = " Center exposure area",
        .onPaint = NULL,
        .result = ACTION_AUTOEXP_CENTER
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 9,
        .caption = " Top",
        .helpcontext = " Top exposure area",
        .onPaint = NULL,
        .result = ACTION_AUTOEXP_TOP
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 9,
        .caption = " Right",
        .helpcontext = " Right exposure area",
        .onPaint = NULL,
        .result = ACTION_AUTOEXP_RIGHT
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 9,
        .caption = " Bottom",
        .helpcontext = " Bottom exposure area",
        .onPaint = NULL,
        .result = ACTION_AUTOEXP_BOTTOM
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 5, .width = 9,
        .caption = " Left",
        .helpcontext = " Left exposure area",
        .onPaint = NULL,
        .result = ACTION_AUTOEXP_LEFT
    }
};
const menu_t AutoexpAreaSubMenu = {
    .x = 5, .y = 4, .width = 11, .height = LENGTH(AutoexpAreaSubMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = AutoexpAreaSubMenuItems, .last_item = LAST_ITEM(AutoexpAreaSubMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


static uint8_t spinedit_gender_value = 0;
const spinedit_value_names_t GenderSpinEditNames[N_GENDER_TYPES] = {
    { .value = gender_type_not_set,     .name = " not set\t" },
    { .value = gender_type_male,        .name = " male\t\t"  },
    { .value = gender_type_female,      .name = " female\t"  }
};
const spinedit_params_t GenderSpinEditParams = {
    .caption = "Gender:",
    .min_value = 0,
    .max_value = MAX_INDEX(GenderSpinEditNames),
    .value = &spinedit_gender_value,
    .names = GenderSpinEditNames,
    .last_name = LAST_ITEM(GenderSpinEditNames)
};
const uint8_t * const GenderNames[N_GENDER_TYPES] = {
    [gender_type_not_set]    = "not set",
    [gender_type_male]       = "male",
    [gender_type_female]     = "felame"
};


static uint8_t spinedit_dither_value = 0;
const spinedit_value_names_t DitherSpinEditNames[N_DITHER_TYPES] = {
    { .value = dither_type_Off,         .name = " Off\t\t"   },
    { .value = dither_type_Default,     .name = " Default\t" },
    { .value = dither_type_2x2,         .name = " 2x2\t\t"   },
    { .value = dither_type_Grid,        .name = " Grid\t\t"  },
    { .value = dither_type_Maze,        .name = " Maze\t\t"  },
    { .value = dither_type_Nest,        .name = " Nest\t\t"  },
    { .value = dither_type_Fuzz,        .name = " Fuzz\t\t"  },
    { .value = dither_type_Vertical,    .name = " Vert.\t\t" },
    { .value = dither_type_Horizonral,  .name = " Horiz.\t\t"},
    { .value = dither_type_Mix,         .name = " Mix\t\t"   }
};
const spinedit_params_t DitherSpinEditParams = {
    .caption = "Dithering:",
    .min_value = 0,
    .max_value = MAX_INDEX(DitherSpinEditNames),
    .value = &spinedit_dither_value,
    .names = DitherSpinEditNames,
    .last_name = LAST_ITEM(DitherSpinEditNames)
};
const uint8_t * const DitherNames[N_DITHER_TYPES] = {
    [dither_type_Off]        = "Off",
    [dither_type_Default]    = "Default",
    [dither_type_2x2]        = "2x2",
    [dither_type_Grid]       = "Grid",
    [dither_type_Maze]       = "Maze",
    [dither_type_Nest]       = "Nest",
    [dither_type_Fuzz]       = "Fuzz",
    [dither_type_Vertical]   = "Vert.",
    [dither_type_Horizonral] = "Horiz.",
    [dither_type_Mix]        = "Mix"
};


uint8_t onExecuteUserInfo(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onTranslateSubResultUserInfo(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onShowUserInfo(const struct menu_t * self, uint8_t * param);
uint8_t * onUserInfoItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onUserInfoMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t UserInfoMenuItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 10, .ofs_y = 7, .width = 4,
        .id = idPopupCameraOwnerOk,
        .caption = ICON_A " Ok",
        .onPaint = NULL,
        .onGetProps = onUserInfoMenuItemProps,
        .result = ACTION_NONE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 5, .ofs_y = 3, .width = 10,
        .id = idPopupCameraOwnerName,
        .caption = "%s",
        .onPaint = onUserInfoItemPaint,
        .onGetProps = onUserInfoMenuItemProps,
        .result = ACTION_OWNER_INFO_NAME
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&GenderSpinEditParams,
        .ofs_x = 5, .ofs_y = 4, .width = 10,
        .id = idPopupCameraOwnerGender,
        .caption = "%s",
        .onPaint = onUserInfoItemPaint,
        .onGetProps = onUserInfoMenuItemProps,
        .result = ACTION_OWNER_INFO_GENDER
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 5, .ofs_y = 5, .width = 10,
        .id = idPopupCameraOwnerBirth,
        .caption = "%s",
        .onPaint = onUserInfoItemPaint,
        .onGetProps = onUserInfoMenuItemProps,
        .result = ACTION_OWNER_INFO_BIRTH
    }
};
const menu_t UserInfoMenu = {
    .x = 2, .y = 4, .width = 16, .height = 9,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_OK,
    .items = UserInfoMenuItems, .last_item = LAST_ITEM(UserInfoMenuItems),
    .onShow = onShowUserInfo, .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultUserInfo, .onExecute = onExecuteUserInfo
};
uint8_t onExecuteUserInfo(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    switch (selection->id) {
        case idPopupCameraOwnerName:
            if (Edit(userinfo_get_username(text_buffer_extra), sizeof(cam_owner_data.user_info.user_name)) == MENU_RESULT_OK) {
                userinfo_set_username(text_buffer_extra);
            }
            return MENU_RESULT_RETURN;
        default:
            break;
    }
    return selection->result;
}
uint8_t onTranslateSubResultUserInfo(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch (self->id) {
        case idPopupCameraOwnerGender:
            if (value == MENU_RESULT_YES) userinfo_set_gender(spinedit_gender_value);
            return MENU_RESULT_RETURN;
        default:
            break;
    }
    return value;
}
uint8_t onUserInfoMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu; self;
    switch (self->id) {
        case idPopupCameraOwnerName:
        case idPopupCameraOwnerGender:
            return ITEM_TEXT_CENTERED;
        case idPopupCameraOwnerBirth:
            return ITEM_DISABLED | ITEM_TEXT_CENTERED;
        default:
            return ITEM_TEXT_CENTERED;
    }
}
uint8_t * onUserInfoItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    switch (self->id) {
        case idPopupCameraOwnerName:
            sprintf(text_buffer, self->caption, userinfo_get_username(text_buffer_extra));
            break;
        case idPopupCameraOwnerGender:
            sprintf(text_buffer, self->caption, GenderNames[userinfo_get_gender()]);
            break;
        case idPopupCameraOwnerBirth:
            sprintf(text_buffer, self->caption, userinfo_get_birthdate(text_buffer_extra));
            break;
        default:
            *text_buffer = 0;
            break;
    }
    return text_buffer;
}
uint8_t onShowUserInfo(const menu_t * self, uint8_t * param) {
    param;
    menu_draw_frame(self);
    menu_draw_shadow(self);

    spinedit_gender_value = userinfo_get_gender();
    if (spinedit_gender_value >= N_GENDER_TYPES) spinedit_gender_value = 0;

    menu_text_out(self->x +  1, self->y,      14, BLACK_ON_WHITE, ITEM_TEXT_CENTERED, "User information:");
    menu_text_out(self->x +  1, self->y +  2,  0, BLACK_ON_WHITE, ITEM_DEFAULT,       "UserID:");
    menu_text_out(self->x +  1, self->y +  3,  0, BLACK_ON_WHITE, ITEM_DEFAULT,       "Name:");
    menu_text_out(self->x +  1, self->y +  4,  0, BLACK_ON_WHITE, ITEM_DEFAULT,       "Gender:");
    menu_text_out(self->x +  1, self->y +  5,  0, BLACK_ON_WHITE, ITEM_DEFAULT,       "Birth:");
    return MENU_PROP_NO_FRAME;
}


uint8_t onTranslateSubResultCameraPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t * onCameraPopupMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onCameraPopupMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t CameraMenuItems[] = {
    {
        .sub = &CameraModeSubMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 13,
        .id = idPopupCameraMode,
        .caption = " Mode\t\t%s",
        .helpcontext = " Select camera mode",
        .onPaint = onCameraPopupMenuItemPaint,
        .onGetProps = onCameraPopupMenuItemProps,
        .result = MENU_RESULT_NONE
    }, {
        .sub = &TriggerSubMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 13,
        .id = idPopupCameraTrigger,
        .caption = " Trigger\t%s",
        .helpcontext = " Trigger behavior",
        .onPaint = onCameraPopupMenuItemPaint,
        .onGetProps = onCameraPopupMenuItemProps,
        .result = MENU_RESULT_NONE
    }, {
        .sub = &ActionSubMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 13,
        .id = idPopupCameraAction,
        .caption = " Action\t\t%s",
        .helpcontext = " Post-processing action",
        .onPaint = onCameraPopupMenuItemPaint,
        .onGetProps = onCameraPopupMenuItemProps,
        .result = MENU_RESULT_NONE
    }, {
        .sub = &AutoexpAreaSubMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 4, .width = 13,
        .id = idPopupCameraArea,
        .caption = " Exp. area\t%s",
        .helpcontext = " Select exposure area",
        .onPaint = onCameraPopupMenuItemPaint,
        .onGetProps = onCameraPopupMenuItemProps,
        .result = MENU_RESULT_NONE
    }, {
        .sub = &SpinEditMenu, .sub_params = (uint8_t *)&DitherSpinEditParams,
        .ofs_x = 1, .ofs_y = 5, .width = 13,
        .id = idPopupCameraDither,
        .caption = " Dithering\t%s",
        .helpcontext = " Select dithering pattern",
        .onPaint = onCameraPopupMenuItemPaint,
        .onGetProps = onCameraPopupMenuItemProps,
        .result = ACTION_SET_DITHERING
    }, {
        .sub = &UserInfoMenu, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 6, .width = 13,
        .id = idPopupCameraOwnerInfo,
        .caption = " Owner information...",
        .helpcontext = " Set camera owner data",
        .onPaint = onCameraPopupMenuItemPaint,
        .onGetProps = onCameraPopupMenuItemProps,
        .result = ACTION_SET_OWNER_INFO
    }, {
        .sub = &YesNoMenu, .sub_params = "Restore defaults?",
        .ofs_x = 1, .ofs_y = 7, .width = 13,
        .id = idPopupCameraRestore,
        .caption = " Restore defaults",
        .helpcontext = " Restore default settings",
        .onPaint = onCameraPopupMenuItemPaint,
        .onGetProps = onCameraPopupMenuItemProps,
        .result = ACTION_RESTORE_DEFAULTS
    }
};
const menu_t CameraPopupMenu = {
    .x = 1, .y = 3, .width = 15, .height = LENGTH(CameraMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = CameraMenuItems, .last_item = LAST_ITEM(CameraMenuItems),
    .onShow = NULL, .onIdle = onIdleCameraPopup, .onHelpContext = onHelpCameraPopup,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultCameraPopup
};
static const menu_item_t * camera_popup_last_selection = NULL;

uint8_t onTranslateSubResultCameraPopup(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    switch (self->id) {
        case idPopupCameraDither:
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
    };
    static const uint8_t * const trigger_modes[N_TRIGGER_MODES] = {
        [trigger_mode_abutton]          = "[" ICON_A " button]",
        [trigger_mode_timer]            = "[Timer]",
        [trigger_mode_repeat]           = "[Repeat]",
        [trigger_mode_AEB]              = "[AEB mode]"
    };
    static const uint8_t * const after_actions[N_AFTER_ACTIONS] = {
        [after_action_save]             = "[Save]",
        [after_action_print]            = "[Print]",
        [after_action_printsave]        = "[S & P]",
        [after_action_transfer]         = "[Transfer]",
        [after_action_transfersave]     = "[S & T]",
        [after_action_picnrec]          = "[Pic'n'Rec]",
        [after_action_printpicnrec]     = "[P & P]",
        [after_action_picnrec_video]    = "[P'n'R " ICON_REC "]",
        [after_action_transfer_video]   = "[Trn " ICON_REC"]",
        [after_action_savesd]           = "[Save to SD]"
    };
    static const uint8_t * const autoexp_areas[N_AUTOEXP_AREAS] = {
        [area_center]                   = "[Center]",
        [area_top]                      = "[Top]",
        [area_right]                    = "[Right]",
        [area_bottom]                   = "[Bottom]",
        [area_left]                     = "[Left]"
    };
    static const uint8_t * const dither_patterns[N_DITHER_TYPES] = {
        [dither_type_Off]               = "[Off]",
        [dither_type_Default]           = "[Default]",
        [dither_type_2x2]               = "[2x2]",
        [dither_type_Grid]              = "[Grid]",
        [dither_type_Maze]              = "[Maze]",
        [dither_type_Nest]              = "[Nest]",
        [dither_type_Fuzz]              = "[Fuzz]",
        [dither_type_Vertical]          = "[Vert.]",
        [dither_type_Horizonral]        = "[Horiz.]",
        [dither_type_Mix]               = "[Mix]"
    };

    switch ((camera_popup_menu_e)self->id) {
        case idPopupCameraRestore:
        case idPopupCameraOwnerInfo:
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
        case idPopupCameraArea:
            sprintf(text_buffer, self->caption, autoexp_areas[OPTION(autoexp_area)]);
            break;
        case idPopupCameraDither:
            sprintf(text_buffer, self->caption, dither_patterns[SETTING(dithering)]);
            break;
        default:
            *text_buffer = 0;
            break;
    }
    return text_buffer;
}
uint8_t onCameraPopupMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    switch ((camera_popup_menu_e)self->id) {
        case idPopupCameraArea:
            return (OPTION(camera_mode) == camera_mode_auto) ? ITEM_DEFAULT : ITEM_DISABLED;
        default:
            return ITEM_DEFAULT;
    }
}
uint8_t onIdleCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection) {
    if (menu == &CameraPopupMenu) camera_popup_last_selection = selection;
    // wait for VBlank if not capturing (avoid HALT CPU state)
    if (!image_is_capturing() && !recording_video) sync_vblank();
    return 0;
}
uint8_t onHelpCameraPopup(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, ITEM_DEFAULT, selection->helpcontext);
    return 0;
}

uint8_t menu_popup_camera_execute(void) BANKED {
    spinedit_timer_value = OPTION(shutter_timer);
    spinedit_counter_value = OPTION(shutter_counter);
    spinedit_aeb_overexp_count = OPTION(aeb_overexp_count);
    spinedit_aeb_exp_step = OPTION(aeb_overexp_step);
    spinedit_dither_value = SETTING(dithering);
    uint8_t menu_result;
    switch (menu_result = menu_execute(&CameraPopupMenu, NULL, camera_popup_last_selection)) {
        case ACTION_TRIGGER_TIMER:
            OPTION(shutter_timer) = spinedit_timer_value;
            break;
        case ACTION_TRIGGER_INTERVAL:
            OPTION(shutter_counter) = spinedit_counter_value;
            break;
        case ACTION_TRIGGER_AEB:
            OPTION(aeb_overexp_count) = spinedit_aeb_overexp_count;
            OPTION(aeb_overexp_step) = spinedit_aeb_exp_step;
            break;
        case ACTION_SET_DITHERING:
            SETTING(dithering) = spinedit_dither_value;
            break;
        default:
            break;
    }
    return menu_result;
}

