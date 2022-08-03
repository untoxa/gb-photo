#ifndef __STATE_CAMERA_H_INCLUDE__
#define __STATE_CAMERA_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdbool.h>
#include "gbcamera.h"
#include "globals.h"
#include "systemdetect.h"
#include "systemhelpers.h"

#define TO_EXPOSURE_VALUE(A) ((uint16_t)((A) >> 4))
#define FROM_EXPOSURE_VALUE(A) (((uint32_t)(A)) << 4)

BANKREF_EXTERN(state_camera)

typedef enum {
    camera_mode_manual,
    camera_mode_assisted,
    camera_mode_auto,
    camera_mode_iterate,
    N_CAMERA_MODES
} camera_mode_e;

typedef enum {
    trigger_mode_abutton,
    trigger_mode_timer,
    trigger_mode_interval,
    N_TRIGGER_MODES
} trigger_mode_e;

typedef enum {
    after_action_save,
    after_action_print,
    after_action_printsave,
    after_action_transfer,
    after_action_transfersave,
    after_action_picnrec,
    after_action_picnrec_video,
    N_AFTER_ACTIONS
} after_action_e;

typedef enum {
    shutter_sound_0,
    shutter_sound_1,
    N_SHUTTER_SOUNDS
} shutter_sound_e;

typedef enum {
    idNone = 0,
    idExposure,
    idGain,
    idVOut,
    idContrast,
    idDither,
    idDitherLight,
    idInvOutput,
    idZeroPoint,
    idVoltageRef,
    idEdgeRatio,
    idEdgeExclusive,
    idEdgeOperation
} camera_menu_e;

typedef struct table_value_t {
    uint8_t value;
    const uint8_t * caption;
} table_value_t;

typedef struct shutter_sound_t {
    uint8_t bank;
    uint8_t * sound;
    uint8_t mask;
} shutter_sound_t;

typedef struct camera_state_options_t {
    camera_mode_e camera_mode;
    trigger_mode_e trigger_mode;
    after_action_e after_action;
    uint8_t gallery_picture_idx;
    uint8_t print_frame_idx;
    bool print_fast             : 1;
    bool fancy_sgb_border       : 1;
    shutter_sound_e shutter_sound;
    uint8_t shutter_timer;
    uint8_t shutter_counter;
} camera_state_options_t;

#define OPTION(OPT) camera_state.OPT
extern camera_state_options_t camera_state;

typedef struct camera_shadow_regs_t {
    uint8_t CAM_REG_CAPTURE;
    uint8_t CAM_REG_EDEXOPGAIN;
    uint16_t CAM_REG_EXPTIME;
    uint8_t CAM_REG_EDRAINVVREF;
    uint8_t CAM_REG_ZEROVOUT;
} camera_shadow_regs_t;

typedef struct camera_mode_settings_t {
    uint16_t current_exposure;
    int8_t current_exposure_idx;
    int8_t current_gain;
    int8_t current_zero_point;
    int8_t current_edge_ratio;
    int8_t current_voltage_ref;
    int16_t voltage_out;
    uint8_t current_contrast;
    uint8_t edge_operation;
    bool dithering              : 1;
    bool ditheringHighLight     : 1;
    bool invertOutput           : 1;
    bool edge_exclusive         : 1;
} camera_mode_settings_t;

typedef struct image_metadata_t {
    camera_shadow_regs_t raw_regs;
    camera_mode_settings_t settings;
    uint16_t crc;
} image_metadata_t;
CHECK_SIZE_NOT_LARGER(image_metadata_t, CAMERA_THUMB_TILE_WIDTH * 4 * 2);   // 4 rows last rows of each last thumbnail tile 2 bytes each row

#define MODE_SETTING(SET,STAT) current_settings[OPTION(STAT)].SET
#define SETTING(SET) MODE_SETTING(SET,camera_mode)
#define CURRENT_SETTINGS current_settings[OPTION(camera_mode)]
extern camera_mode_settings_t current_settings[N_CAMERA_MODES];

extern uint8_t recording_video;

extern camera_shadow_regs_t SHADOW;

inline uint8_t is_capturing() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    return (CAM_REG_CAPTURE & CAM00F_CAPTURING);
}
inline uint8_t image_captured() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    uint8_t v = CAM_REG_CAPTURE;
    uint8_t r = (((v ^ SHADOW.CAM_REG_CAPTURE) & CAM00F_CAPTURING) && !(v & CAM00F_CAPTURING));
    SHADOW.CAM_REG_CAPTURE = v;
    return r;
}
inline void image_capture() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    SHADOW.CAM_REG_CAPTURE = CAM_REG_CAPTURE = (CAM00F_POSITIVE | CAM00F_CAPTURING);
}

uint8_t * camera_format_item_text(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings) BANKED;

#endif
