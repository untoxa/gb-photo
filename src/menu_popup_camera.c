#pragma bank 255

#include <gbdk/platform.h>
#include <stdio.h>

#include "globals.h"

// camera state
#include "state_camera.h"

// menus
#include "menus.h"
#include "menu_codes.h"

const uint8_t * const camera_modes[] =  {"[Manual]", "[Assist]", "[Auto]", "[Iter]"};
const uint8_t * const trigger_modes[] = {"[Btn: A]", "[Timer]", "[Repeat]"};
const uint8_t * const after_actions[] = {"[Save]", "[Print]", "[S & P]"};

const menu_item_t ModeSubMenuItemManual = {
    .prev = NULL,                       .next = &ModeSubMenuItemAssisted, 
    .sub = NULL, .sub_params = NULL,        
    .ofs_x = 1, .ofs_y = 1, .width = 11, 
    .caption = " Manual",
    .onPaint = NULL,
    .result = ACTION_MODE_MANUAL
};
const menu_item_t ModeSubMenuItemAssisted = {
    .prev = &ModeSubMenuItemManual,     .next = &ModeSubMenuItemAuto, 
    .sub = NULL, .sub_params = NULL,        
    .ofs_x = 1, .ofs_y = 2, .width = 11, 
    .caption = " Assisted",
    .onPaint = NULL,
    .result = ACTION_MODE_ASSISTED
};
const menu_item_t ModeSubMenuItemAuto = {
    .prev = &ModeSubMenuItemAssisted,   .next = &ModeSubMenuItemIterate, 
    .sub = NULL, .sub_params = NULL,        
    .ofs_x = 1, .ofs_y = 3, .width = 11, 
    .caption = " Auto",
    .onPaint = NULL,
    .result = ACTION_MODE_AUTO
};
const menu_item_t ModeSubMenuItemIterate = {
    .prev = &ModeSubMenuItemAuto,       .next = NULL, 
    .sub = NULL, .sub_params = NULL,        
    .ofs_x = 1, .ofs_y = 4, .width = 11, 
    .caption = " Iterate (Bracket)",
    .onPaint = NULL,
    .result = ACTION_MODE_ITERATE
};
const menu_t CameraModeSubMenu = {
    .x = 5, .y = 4, .width = 13, .height = 6, 
    .items = &ModeSubMenuItemManual, 
    .onShow = NULL, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};


const menu_item_t TriggerSubMenuItemAButton = {
    .prev = NULL,                       .next = &TriggerSubMenuItemTimer, 
    .sub = NULL, .sub_params = NULL,        
    .ofs_x = 1, .ofs_y = 1, .width = 8, 
    .caption = " [A] Button",
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
    .items = &TriggerSubMenuItemAButton, 
    .onShow = NULL, .onTranslateKey = NULL, .onTranslateSubResult = NULL
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
    .items = &ActionSubMenuSave, 
    .onShow = NULL, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t CameraMenuItemMode = {
    .prev = NULL,                   .next = &CameraMenuItemTrigger, 
    .sub = &CameraModeSubMenu, .sub_params = NULL,        
    .ofs_x = 1, .ofs_y = 1, .width = 11, 
    .caption = " Mode\t\t%s",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemTrigger = {
    .prev = &CameraMenuItemMode,  .next = &CameraMenuItemAction,
    .sub = &TriggerSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 11, 
    .caption = " Trigger\t%s",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_item_t CameraMenuItemAction = {
    .prev = &CameraMenuItemTrigger, .next = NULL,
    .sub = &ActionSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 11, 
    .caption = " Action\t\t%s",
    .onPaint = onCameraMenuItemPaint,
    .result = MENU_RESULT_NONE
};
const menu_t CameraPopupMenu = {
    .x = 1, .y = 3, .width = 13, .height = 5, 
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &CameraMenuItemMode, 
    .onShow = NULL, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};
uint8_t * onCameraMenuItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    if (self == &CameraMenuItemMode) {
        sprintf(text_buffer, self->caption, camera_modes[camera_mode]);
    } else if (self == &CameraMenuItemTrigger) {
        sprintf(text_buffer, self->caption, trigger_modes[trigger_mode]);
    } else if (self == &CameraMenuItemAction) {
        sprintf(text_buffer, self->caption, after_actions[after_action]);
    } else *text_buffer = 0;
    return text_buffer;
}
/*
    menu_text_out( 0, 0, 5, SOLID_WHITE, camera_modes[camera_mode]);
    menu_text_out( 5, 0, 5, SOLID_WHITE, trigger_modes[trigger_mode]);
    menu_text_out(10, 0, 5, SOLID_WHITE, after_actions[after_action]);
*/


uint8_t menu_popup_camera_execute() BANKED {
    return menu_execute(&CameraPopupMenu, NULL);
}
