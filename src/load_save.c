#pragma bank 255

#include <gbdk/platform.h>
#include <stdbool.h>
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

static save_structure_t AT(0xC000 - sizeof(save_structure_t)) save_structure;  // bind the structure to the top of SRAM bank

const camera_state_options_t default_camera_state_options = {
    .camera_mode = camera_mode_auto,
    .trigger_mode = trigger_mode_abutton,
    .after_action = after_action_save,
    .gallery_picture_idx = 0,
    .print_frame_idx = 0,
    .print_fast = false,
    .shutter_sound = shutter_sound_0,
    .shutter_timer = 10,
    .shutter_counter = 5
};

const camera_mode_settings_t default_camera_mode_settings[N_CAMERA_MODES] = {
    {
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_ratio = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = true, .ditheringHighLight = true, .current_contrast = 9, .invertOutput = false,
        .edge_exclusive = true
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_ratio = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = true, .ditheringHighLight = true, .current_contrast = 9, .invertOutput = false,
        .edge_exclusive = true
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_ratio = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = true, .ditheringHighLight = true, .current_contrast = 9, .invertOutput = false,
        .edge_exclusive = true
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = 14, .current_gain = 0, .current_zero_point = 1, .current_edge_ratio = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = true, .ditheringHighLight = true, .current_contrast = 9, .invertOutput = false,
        .edge_exclusive = true
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

inline void save_wait_sram() {
    SWITCH_RAM(CAMERA_BANK_REGISTERS);
    while (CAM_REG_CAPTURE & CAM00F_CAPTURING);
}

void save_camera_mode_settings(camera_mode_e mode) BANKED {
    save_wait_sram();
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    save_structure.mode_settings[mode] = current_settings[mode];
}

void restore_default_mode_settings(camera_mode_e mode) BANKED {
    save_wait_sram();
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    current_settings[mode] = save_structure.mode_settings[mode] = default_camera_mode_settings[mode];
}

void save_camera_state() BANKED {
    save_wait_sram();
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    save_structure.state_options = camera_state;
}