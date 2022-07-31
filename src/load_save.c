#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "globals.h"
#include "systemhelpers.h"
#include "state_camera.h"
#include "load_save.h"

#define MAGIC_VALUE 0x45564153

#define FREE_SAVE_SPACE 72

typedef struct save_structure_t {
    uint32_t MAGIC;
    camera_state_options_t state_options;
    camera_mode_settings_t mode_settings[N_CAMERA_MODES];
} save_structure_t;
CHECK_SIZE_NOT_LARGER(save_structure_t, FREE_SAVE_SPACE);   // compiling breaks here if sizeof(save_structure_t) becomes larger than the available amount of bytes

save_structure_t AT(0xC000 - sizeof(save_structure_t)) save_structure;  // bind the structure to the top of SRAM bank

const camera_state_options_t default_camera_state_options = {
    .camera_mode = camera_mode_manual,
    .trigger_mode = trigger_mode_abutton,
    .after_action = after_action_save,
    .gallery_picture_idx = 0,
    .print_frame_idx = 0,
    .print_fast = FALSE,
    .shutter_sound = shutter_sound_0
};

const camera_mode_settings_t default_camera_mode_settings[N_CAMERA_MODES] = {
    {
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_mode = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = TRUE, .ditheringHighLight = TRUE, .current_contrast = 9, .invertOutput = FALSE,
        .edge_exclusive = TRUE
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_mode = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = TRUE, .ditheringHighLight = TRUE, .current_contrast = 9, .invertOutput = FALSE,
        .edge_exclusive = TRUE
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_mode = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = TRUE, .ditheringHighLight = TRUE, .current_contrast = 9, .invertOutput = FALSE,
        .edge_exclusive = TRUE
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_mode = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = TRUE, .ditheringHighLight = TRUE, .current_contrast = 9, .invertOutput = FALSE,
        .edge_exclusive = TRUE
    }
};

void init_save_structure() BANKED {
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    if (save_structure.MAGIC != MAGIC_VALUE) {
        save_structure.MAGIC = MAGIC_VALUE;
        save_structure.state_options = default_camera_state_options;
        memcpy(save_structure.mode_settings, default_camera_mode_settings, sizeof(save_structure.mode_settings));
    }
    camera_state = save_structure.state_options;
    memcpy(current_settings, save_structure.mode_settings, sizeof(current_settings));
}

void save_camera_mode_settings(camera_mode_e mode) BANKED {
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    save_structure.mode_settings[mode] = current_settings[mode];
}

void restore_default_mode_settings(camera_mode_e mode) BANKED {
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    current_settings[mode] = save_structure.mode_settings[mode] = default_camera_mode_settings[mode];
}

void save_camera_state() BANKED {
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    save_structure.state_options = camera_state;
}