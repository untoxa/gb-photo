#ifndef __FLIP_H_INCLUDE__
#define __FLIP_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdbool.h>

typedef enum {
    camera_flip_none,
    camera_flip_xy,
    camera_flip_x
} camera_flip_e;

uint8_t * get_flipped_last_seen_image(camera_flip_e flip, bool copy) BANKED;

#endif