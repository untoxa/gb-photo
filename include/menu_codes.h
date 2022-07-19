#ifndef __MENU_CODES_H_INCLUDE__
#define __MENU_CODES_H_INCLUDE__


#define MENU_RESULT_NONE            0
#define MENU_RESULT_CLOSE           1

// default actions
#define ACTION_NONE MENU_RESULT_CLOSE
// main menu actions
#define ACTION_ERASE_GALLERY        10

// gallery actions
#define ACTION_ERASE_IMAGE          20
#define ACTION_DISPLAY_INFO         21
#define ACTION_PRINT_IMAGE          22
#define ACTION_PRINT_FRAME0         23
#define ACTION_PRINT_FRAME1         24
#define ACTION_PRINT_FRAME2         25
#define ACTION_PRINT_FRAME3         26

// state switching actions
#define ACTION_THUMBNAILS           30
#define ACTION_GALLERY              31
#define ACTION_CAMERA               32

// camera popup menu actions
#define ACTION_MODE_MANUAL          40
#define ACTION_MODE_ASSISTED        41
#define ACTION_MODE_AUTO            42
#define ACTION_MODE_ITERATE         43
#define ACTION_TRIGGER_ABUTTON      44
#define ACTION_TRIGGER_TIMER        45
#define ACTION_TRIGGER_INTERVAL     46
#define ACTION_ACTION_SAVE          47
#define ACTION_ACTION_PRINT         48
#define ACTION_ACTION_SAVEPRINT     49
#define ACTION_ACTION_PICNREC       50
#define ACTION_ACTION_PICNREC_VIDEO 51
#define ACTION_RESTORE_DEFAULTS     52

// camera actions
#define ACTION_CAMERA_PRINT         60
#define ACTION_CAMERA_SUBMENU       61

// global actions
#define ACTION_MAIN_MENU            100

// system menu results
#define MENU_RESULT_YES             254
#define MENU_RESULT_NO              253
#define MENU_RESULT_OK              255

#endif