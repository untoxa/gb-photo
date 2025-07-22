#ifndef __FLIP_H_INCLUDE__
#define __FLIP_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdbool.h>

BANKREF_EXTERN(module_flip)

typedef enum {
    camera_flip_none,
    camera_flip_xy,
    camera_flip_x
} camera_flip_e;

uint8_t * get_flipped_last_seen_image(camera_flip_e flip, bool copy) BANKED;

uint8_t INIT_module_flip(void) BANKED;

#endif