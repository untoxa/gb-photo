#ifndef __STATE_CAMERA_H_INCLUDE__
#define __STATE_CAMERA_H_INCLUDE__

#include <gbdk/platform.h>
#include "gbcamera.h"
#include "globals.h"
#include "systemdetect.h"

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
    idDither,
    idInvOutput,
    idZeroPoint,
    idVoltageRef,
    idEdgeMode,
    idEdgeExclusive
} camera_menu_e;

typedef enum {
    changeNone = 0,
    changeDecrease,
    changeIncrease
} change_direction_e;

inline uint8_t inc_dec_int8(int8_t * value, int8_t delta, int8_t min, int8_t max, change_direction_e dir) {
    int8_t v = *value;
    switch (dir) {
        case changeDecrease:
            return (v != (*value = ((*value - delta) < min) ? min : (*value - delta))); 
        case changeIncrease:
            return (v != (*value = ((*value + delta) > max) ? max : (*value + delta))); 
        default:
            return FALSE;
    }
}
inline uint8_t inc_dec_int16(int16_t * value, int16_t delta, int16_t min, int16_t max, change_direction_e dir) {
    int16_t v = *value;
    switch (dir) {
        case changeDecrease:
            *value = ((*value - delta) < min) ? min : (*value - delta); 
            break;
        case changeIncrease:
            *value = ((*value + delta) > max) ? max : (*value + delta); 
            break;
    }
    return (v != *value);
}

typedef struct table_value_t {
    uint8_t value;
    const uint8_t * caption;
} table_value_t;

extern camera_mode_e camera_mode;
extern trigger_mode_e trigger_mode;
extern after_action_e after_action;

extern uint8_t old_capture_reg;

inline uint8_t is_capturing() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    return (CAM_REG_CAPTURE & CAM00F_CAPTURING);
}
inline uint8_t image_captured() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    uint8_t v = CAM_REG_CAPTURE;
    uint8_t r = (((v ^ old_capture_reg) & CAM00F_CAPTURING) && !(v & CAM00F_CAPTURING));
    old_capture_reg = v;
#if (USE_CGB_DOUBLE_SPEED==1)    
    if (r) cpu_fast();              // speed up when captured
#endif
    return r;
}
inline void image_capture() {
#if (USE_CGB_DOUBLE_SPEED==1)    
    if (_is_COLOR) cpu_slow();      // slowdown before capturing image
#endif
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    old_capture_reg = CAM_REG_CAPTURE = (CAM00F_POSITIVE | CAM00F_CAPTURING);
}


#endif