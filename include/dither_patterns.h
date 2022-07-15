#ifndef __DITHER_PATTERNS_H_INCLUDE__
#define __DITHER_PATTERNS_H_INCLUDE__

#define NUM_ONOFF_SETS 2
#define NUM_LIGHT_SETS 2
#define NUM_CONTRAST_SETS 16
#define NUM_CONTRAST_SIZE 48

void dither_pattern_apply(uint8_t dithering, uint8_t ditheringHighLight, uint8_t current_contrast) BANKED;

#endif