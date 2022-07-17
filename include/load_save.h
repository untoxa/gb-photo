#ifndef __LOAD_SAVE_H_INCLUDE__
#define __LOAD_SAVE_H_INCLUDE__

#include "state_camera.h"

void init_save_structure() BANKED;
void save_camera_mode_settings(camera_mode_e mode) BANKED;
void restore_default_mode_settings(camera_mode_e mode) BANKED;
void save_camera_state() BANKED;

#endif