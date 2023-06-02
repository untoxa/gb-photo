#ifndef __HISTOGRAM_H_INCLUDE__
#define __HISTOGRAM_H_INCLUDE__

#include <stdint.h>

#define HISTOGRAM_TARGET_VALUE ((int16_t)&__histogram_target_value)
#define HISTOGRAM_MAX_VALUE ((int16_t)&__histogram_max_value)
#define HISTOGRAM_POINTS_COUNT ((uint8_t)&__histogram_points_count)

extern uint16_t histogram[4];

extern void __histogram_target_value;
extern void __histogram_max_value;
extern void __histogram_points_count;

int16_t calculate_histogram(void) BANKED;

#endif