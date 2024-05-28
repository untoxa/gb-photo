#ifndef __MODE_SLITSCAN_H_INCLUDE__
#define __MODE_SLITSCAN_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    slitscan_delay_none = 0,
    slitscan_delay_0_05_sec,
    slitscan_delay_0_10_sec,
    slitscan_delay_0_25_sec,
    slitscan_delay_0_50_sec,
    slitscan_delay_1_00_sec,
    N_SLITSCAN_DELAYS
} slitscan_delay_e;

typedef enum {
    slitscan_MotionTrigger_none = 0,
    slitscan_MotionTrigger_50,
    slitscan_MotionTrigger_100,
    slitscan_MotionTrigger_200,
    slitscan_MotionTrigger_400,
    slitscan_MotionTrigger_800,
    slitscan_MotionTrigger_1600,
    N_SLITSCAN_MOTIONTRIGGERS
} slitscan_motiontrigger_e;


#define SECOND_TO_FRAMES(sec) ((uint8_t)(60 * sec))
extern const uint8_t slitscan_delays[N_SLITSCAN_DELAYS];
extern const uint16_t slitscan_motiontriggers[N_SLITSCAN_MOTIONTRIGGERS];


#define SLITSCAN_STATE_STILL_CAPTURING  0
#define SLITSCAN_STATE_DONE             1

bool slitscan_is_capturing(void) NONBANKED;
void slitscan_mode_on_trigger(void) BANKED;
uint8_t slitscan_mode_on_image_captured(void) BANKED;

#endif