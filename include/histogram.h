#ifndef __HISTOGRAM_H_INCLUDE__
#define __HISTOGRAM_H_INCLUDE__

#include <stdint.h>

extern uint16_t histogram[4];
extern int16_t histogram_target_value;     // not a constant just to not deal with banks
extern uint8_t histogram_points_count;     // not a constant just to not deal with banks

int16_t calculate_histogram() BANKED;

#endif