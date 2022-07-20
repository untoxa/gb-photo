#ifndef FADE_MANAGER_H
#define FADE_MANAGER_H

#include <gbdk/platform.h>
#include <stdint.h>

#define FADE_SPEED_MASK 0x3F
#define FADE_IN_FLAG 0x40
#define FADE_ENABLED_FLAG 0x80

typedef enum { FADE_IN, FADE_OUT } FADE_DIRECTION;

extern uint8_t fade_running;
extern uint8_t fade_frames_per_step;
extern uint8_t fade_style;

/**
 * Initialise fade variables
 */
void fade_init() BANKED;

/**
 * Change current fade speed
 *
 * @param speed new fade speed
 */
void fade_setspeed(uint8_t speed) BANKED;

/**
 * Fade in and wait until complete
 */
void fade_in_modal() BANKED;

/**
 * Fade out and wait until complete
 */
void fade_out_modal() BANKED;

#endif
