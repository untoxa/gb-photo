#ifndef __LOAD_SAVE_H_INCLUDE__
#define __LOAD_SAVE_H_INCLUDE__

#include "state_camera.h"

BANKREF_EXTERN(module_load_save)

#define LOAD_SAVE_DATA_BANK 1

#define MAGIC_VALUE 0x45564153

#define FREE_SAVE_SPACE 72

typedef struct save_structure_t {
    uint32_t MAGIC;
    camera_state_options_t state_options;
    camera_mode_settings_t mode_settings[N_CAMERA_MODES];
} save_structure_t;
CHECK_SIZE_NOT_LARGER(save_structure_t, FREE_SAVE_SPACE);   // compiling breaks here if sizeof(save_structure_t) becomes larger than the available amount of bytes

void save_camera_mode_settings(camera_mode_e mode) BANKED;
void restore_default_mode_settings(camera_mode_e mode) BANKED;
void save_camera_state() BANKED;

uint8_t INIT_module_load_save() BANKED;

#endif