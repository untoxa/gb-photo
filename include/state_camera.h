#ifndef __STATE_CAMERA_H_INCLUDE__
#define __STATE_CAMERA_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdbool.h>

#include "compat.h"
#include "gbcamera.h"
#include "globals.h"

#include "systemdetect.h"
#include "systemhelpers.h"

#define TO_EXPOSURE_VALUE(A) ((uint16_t)((A) >> 4))
#define FROM_EXPOSURE_VALUE(A) (((uint32_t)(A)) << 4)

#define EXPOSURE_LOW_LIMIT TO_EXPOSURE_VALUE(256)
#define EXPOSURE_HIGH_LIMIT CAM02_MAX_VALUE

BANKREF_EXTERN(state_camera)

typedef enum {
    camera_mode_manual,
    camera_mode_assisted,
    camera_mode_auto,
    N_CAMERA_MODES
} camera_mode_e;

typedef enum {
    trigger_mode_abutton,
    trigger_mode_timer,
    trigger_mode_repeat,
    trigger_mode_AEB,
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
    after_action_transfer_video,
    after_action_savesd,
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
    idEdgeOperation,
    idBrightness
} camera_menu_e;

typedef enum {
    area_center = 0,
    area_top,
    area_right,
    area_bottom,
    area_left,
    N_AUTOEXP_AREAS
} autoexp_area_e;

typedef enum {
    cart_type_HDR = 0,
    cart_type_iG_AIO,
    N_CART_TYPES
} cart_type_e;

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
    camera_mode_e camera_mode       : 4;
    trigger_mode_e trigger_mode     : 4;
    after_action_e after_action     : 4;
    shutter_sound_e shutter_sound   : 4;
    uint8_t gallery_picture_idx;
    uint8_t print_frame_idx;
    bool print_fast                 : 1;
    bool fancy_sgb_border           : 1;
    bool show_grid                  : 1;
    bool save_confirm               : 1;
    bool ir_remote_shutter          : 1;
    bool boot_to_camera_mode        : 1;
    bool flip_live_view             : 1;
    bool double_speed               : 1;
    uint8_t shutter_timer;
    uint8_t shutter_counter;
    uint8_t cgb_palette_idx         : 4;
    bool display_exposure           : 1;
    bool enable_DMA                 : 1;
    uint8_t aeb_overexp_count;
    uint8_t aeb_overexp_step;
    autoexp_area_e autoexp_area     : 4;
    cart_type_e cart_type           : 4;
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
    int16_t current_brightness;
    uint8_t dithering           : 4;
    bool ditheringHighLight     : 1;
    bool invertOutput           : 1;
    bool edge_exclusive         : 1;
    bool cpu_fast               : 1;
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

extern bool recording_video;

extern camera_shadow_regs_t SHADOW;

#define PNR_DELAY_FRAMES 6
extern volatile uint8_t camera_PnR_delay;

inline void set_image_refresh_dalay(uint8_t delay) {
    camera_PnR_delay = delay;
}
inline bool image_is_capturing(void) {
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    return ((camera_PnR_delay) || (CAM_REG_CAPTURE & CAM00F_CAPTURING));
}

uint8_t * camera_format_item_text(camera_menu_e id, const uint8_t * format, camera_mode_settings_t * settings) BANKED;

#define COUNTER_INFINITE_VALUE 31

#define DEFAULT_CONTRAST_VALUE 9
#define DEFAULT_EXPOSURE_INDEX 28

#define MAX_AEB_IMAGES 29
#define MAX_AEB_OVEREXPOSURE (MAX_AEB_IMAGES >> 1)
#define MIDDLE_AEB_IMAGE (MAX_AEB_IMAGES >> 1)

#endif
