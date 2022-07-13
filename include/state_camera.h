#ifndef __STATE_CAMERA_H_INCLUDE__
#define __STATE_CAMERA_H_INCLUDE__

typedef enum {
    camera_mode_manual,
    camera_mode_assisted,
    camera_mode_auto,
    camera_mode_iterate  
} camera_mode_e;

typedef enum {
    trigger_mode_abutton,
    trigger_mode_timer,
    trigger_mode_interval
} trigger_mode_e;

typedef enum {
    after_action_save,
    after_action_print,
    after_action_printsave
} after_action_e;

#endif