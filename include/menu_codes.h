#ifndef __MENU_CODES_H_INCLUDE__
#define __MENU_CODES_H_INCLUDE__

// #define MENU_RESULT_NONE            0    // defined in menus.h
#define MENU_RESULT_CLOSE           1

// default actions
#define ACTION_NONE MENU_RESULT_CLOSE

// gallery actions
#define ACTION_ERASE_GALLERY        10
#define ACTION_ERASE_IMAGE          11
#define ACTION_UNERASE_GALLERY      12
#define ACTION_TRANSFER_IMAGE       13
#define ACTION_TRANSFER_GALLERY     14
#define ACTION_PRINT_IMAGE          15
#define ACTION_PRINT_GALLERY        16
#define ACTION_PRINT_INFO           17

// settings
#define ACTION_SETTINGS_PRINT_FAST  20
#define ACTION_SETTINGS_ALT_BORDER  21
#define ACTION_SETTINGS_CGB_PALETTE 22
#define ACTION_SETTINGS_SHOW_GRID   23
#define ACTION_SETTINGS_SAVE_CONF   24
#define ACTION_SETTINGS_IR_REMOTE   25
#define ACTION_SETTINGS_BOOT_TO_CAM 26
#define ACTION_SETTINGS_FLIP_IMAGE  27
#define ACTION_SETTINGS_DOUBLESPEED 28
#define ACTION_SETTINGS_DISPLAY_EXP 29

// state switching actions
#define ACTION_GALLERY              30
#define ACTION_CAMERA               31
#define ACTION_SETTINGS             32
#define ACTION_FLASHER              33

// camera popup menu actions
#define ACTION_MODE_MANUAL          40
#define ACTION_MODE_ASSISTED        41
#define ACTION_MODE_AUTO            42
#define ACTION_TRIGGER_ABUTTON      43
#define ACTION_TRIGGER_TIMER        44
#define ACTION_TRIGGER_INTERVAL     45
#define ACTION_TRIGGER_AEB          46
#define ACTION_ACTION_SAVE          47
#define ACTION_ACTION_PRINT         48
#define ACTION_ACTION_SAVEPRINT     49
#define ACTION_ACTION_TRANSFER      50
#define ACTION_ACTION_SAVETRANSFER  51
#define ACTION_ACTION_PICNREC       52
#define ACTION_ACTION_PICNREC_VIDEO 53
#define ACTION_ACTION_TRANSF_VIDEO  54
#define ACTION_RESTORE_DEFAULTS     55

// camera actions
#define ACTION_CAMERA_PRINT         60
#define ACTION_CAMERA_TRANSFER      61
#define ACTION_CAMERA_SUBMENU       62

// thumbnail actions
#define ACTION_DELETE_SELECTED      70
#define ACTION_PRINT_SELECTED       71
#define ACTION_TRANSFER_SELECTED    72

// flasher
#define ACTION_FLASH_SAVE           80
#define ACTION_FLASH_MOVE           81
#define ACTION_FLASH_LOAD           82
#define ACTION_FLASH_ERASE          83
#define ACTION_PRINT_SLOT           84
#define ACTION_TRANSFER_SLOT        85
#define ACTION_RESTORE_IMAGE        86

// printer frames
#define ACTION_PRINT_FRAME0         90
#define ACTION_PRINT_FRAME1         91
#define ACTION_PRINT_FRAME2         92
#define ACTION_PRINT_FRAME3         93
#define ACTION_PRINT_FRAME4         94
#define ACTION_PRINT_FRAME5         95
#define ACTION_PRINT_FRAME6         96
#define ACTION_PRINT_FRAME7         97
#define ACTION_PRINT_FRAME8         98
#define ACTION_PRINT_FRAME9         99
#define ACTION_PRINT_FRAME10        100
#define ACTION_PRINT_FRAME11        101

#define ACTION_PRINT_FRAME_FIRST    ACTION_PRINT_FRAME0
#define ACTION_PRINT_FRAME_LAST     ACTION_PRINT_FRAME11

// global actions
#define ACTION_MAIN_MENU            150
#define ACTION_ABOUT                151

// system menu results
#define MENU_RESULT_NO              253
#define MENU_RESULT_YES             254
#define MENU_RESULT_OK              255

#endif