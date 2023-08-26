#pragma bank 255

#include <gbdk/platform.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "globals.h"
#include "systemhelpers.h"
#include "state_camera.h"
#include "load_save.h"
#include "histogram.h"

BANKREF(module_load_save)

#define LOAD_SAVE_OFFSET (0xC000 - sizeof(save_structure_t))

static save_structure_t AT(LOAD_SAVE_OFFSET) save_structure;  // bind the structure to the top of SRAM bank

const camera_state_options_t default_camera_state_options = {
    .camera_mode = camera_mode_auto,
    .trigger_mode = trigger_mode_abutton,
    .after_action = after_action_save,
    .gallery_picture_idx = 0,
    .print_frame_idx = 0,
    .print_fast = false,
    .fancy_sgb_border = false,
    .show_grid = false,
    .save_confirm = true,
    .ir_remote_shutter = false,
    .boot_to_camera_mode = false,
    .flip_live_view = false,
    .double_speed = false,
    .shutter_sound = shutter_sound_0,
    .shutter_timer = 10,
    .shutter_counter = 5,
    .cgb_palette_idx = 0,
    .display_exposure = false,
    .enable_DMA = false,
    .aeb_overexp_count = 4,
    .aeb_overexp_step = 0,
    .autoexp_area = area_center
};

const camera_mode_settings_t default_camera_mode_settings[N_CAMERA_MODES] = {
    {
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = DEFAULT_EXPOSURE_INDEX, .current_gain = 0, .current_zero_point = 1, .current_edge_ratio = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = 1, .ditheringHighLight = true, .current_contrast = DEFAULT_CONTRAST_VALUE, .invertOutput = false,
        .current_brightness = HISTOGRAM_TARGET_VALUE,
        .edge_exclusive = true
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = DEFAULT_EXPOSURE_INDEX, .current_gain = 0, .current_zero_point = 1, .current_edge_ratio = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = 1, .ditheringHighLight = true, .current_contrast = DEFAULT_CONTRAST_VALUE, .invertOutput = false,
        .current_brightness = HISTOGRAM_TARGET_VALUE,
        .edge_exclusive = true
    },{
        .current_exposure = TO_EXPOSURE_VALUE(6000),
        .current_exposure_idx = DEFAULT_EXPOSURE_INDEX, .current_gain = 0, .current_zero_point = 1, .current_edge_ratio = 0, .current_voltage_ref = 3,
        .voltage_out = 192, .dithering = 1, .ditheringHighLight = true, .current_contrast = DEFAULT_CONTRAST_VALUE, .invertOutput = false,
        .current_brightness = HISTOGRAM_TARGET_VALUE,
        .edge_exclusive = true
    }
};

inline void save_wait_sram(void) {
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

void save_camera_state(void) BANKED {
    save_wait_sram();
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    save_structure.state_options = camera_state;
}

bool camera_settings_reset = false;

// enable battery backed-up SRAM and load/initialize program settings
uint8_t INIT_module_load_save(void) BANKED {
    ENABLE_RAM;
    SWITCH_RAM(LOAD_SAVE_DATA_BANK);
    // check for the valid save blob and initialize with defaults if fail
    if (save_structure.MAGIC != MAGIC_VALUE) {
        save_structure.MAGIC = MAGIC_VALUE;
        save_structure.state_options = default_camera_state_options;
        memcpy(save_structure.mode_settings, default_camera_mode_settings, sizeof(save_structure.mode_settings));
        camera_settings_reset = true;
    }
    // load camera state
    camera_state = save_structure.state_options;
    memcpy(current_settings, save_structure.mode_settings, sizeof(current_settings));
    // apply hardware settings
    if (_is_COLOR) {
        if (OPTION(double_speed)) CPU_FAST(); else CPU_SLOW();
    }
    return 0;
}