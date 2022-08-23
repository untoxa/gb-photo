#ifndef __LOAD_SAVE_H_INCLUDE__
#define __LOAD_SAVE_H_INCLUDE__

#include "state_camera.h"

BANKREF_EXTERN(module_load_save)

#define LOAD_SAVE_DATA_BANK 1

void save_camera_mode_settings(camera_mode_e mode) BANKED;
void restore_default_mode_settings(camera_mode_e mode) BANKED;
void save_camera_state() BANKED;

uint8_t INIT_module_load_save() BANKED;

#endif