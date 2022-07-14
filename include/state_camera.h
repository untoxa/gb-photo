#ifndef __STATE_CAMERA_H_INCLUDE__
#define __STATE_CAMERA_H_INCLUDE__

#include "gbcamera.h"

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

typedef enum {
    idNone = 0, 
    idExposure,
    idGain,
    idVOut,
    idContrast,
    idDither
} camera_menu_e;

typedef enum {
    changeNone = 0,
    changeDecrease,
    changeIncrease
} change_direction_e;

extern camera_mode_e camera_mode;
extern trigger_mode_e trigger_mode;
extern after_action_e after_action;

extern uint8_t old_capture_reg;

inline uint8_t is_capturing() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    return (CAM_REG_CAPTURE & CAPTF_CAPTURING);
}
inline uint8_t image_captured() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    uint8_t v = CAM_REG_CAPTURE;
    uint8_t r = (((v ^ old_capture_reg) & CAPTF_CAPTURING) && !(v & CAPTF_CAPTURING));
    old_capture_reg = v;
    return r;
}
inline void image_capture(uint8_t capture) {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    old_capture_reg = CAM_REG_CAPTURE = capture;
}


#endif