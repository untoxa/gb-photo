#ifndef __MENU_CODES_H_INCLUDE__
#define __MENU_CODES_H_INCLUDE__


#define MENU_RESULT_NONE        0
#define MENU_RESULT_CLOSE       1

// default actions
#define ACTION_NONE MENU_RESULT_CLOSE
// main menu actions
#define ACTION_ERASE_GALLERY    2
// gallery actions
#define ACTION_ERASE_IMAGE      3
#define ACTION_DISPLAY_INFO     4
#define ACTION_PRINT_FRAME0     5
#define ACTION_PRINT_FRAME1     6
#define ACTION_PRINT_FRAME2     7
#define ACTION_PRINT_FRAME3     8

// state switching actions
#define ACTION_THUMBNAILS       9
#define ACTION_GALLERY          10
#define ACTION_CAMERA           11

// camera popup menu actions
#define ACTION_MODE_MANUAL      12
#define ACTION_MODE_ASSISTED    13
#define ACTION_MODE_AUTO        14
#define ACTION_MODE_ITERATE     15
#define ACTION_TRIGGER_ABUTTON  16
#define ACTION_TRIGGER_TIMER    17
#define ACTION_TRIGGER_INTERVAL 18
#define ACTION_ACTION_SAVE      19
#define ACTION_ACTION_PRINT     20
#define ACTION_ACTION_SAVEPRINT 21
#define ACTION_RESTORE_DEFAULTS 22

// camera actions
#define ACTION_SHUTTER          23
#define ACTION_CAMERA_SUBMENU   24

// system menu results
#define MENU_RESULT_YES         254
#define MENU_RESULT_NO          253
#define MENU_RESULT_OK          255


#endif