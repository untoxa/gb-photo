#ifndef __DITHER_PATTERNS_H_INCLUDE__
#define __DITHER_PATTERNS_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define NUM_INTERVALS 3
#define NUM_CONTRAST_VALUES 16
#define NUM_BASE_VALUES 16
#define PATTERN_MATRIX_SIZE (4 * 4)

typedef enum {
    dither_type_Off,
    dither_type_Default,
    dither_type_2x2,
    dither_type_Grid,
    dither_type_Maze,
    dither_type_Nest,
    dither_type_Fuzz,
    dither_type_Vertical,
    dither_type_Horizonral,
    dither_type_Mix,
    N_DITHER_TYPES
} dither_type_e;

void dither_pattern_apply(uint8_t dithering, uint8_t ditheringHighLight, uint8_t current_contrast) BANKED;

#endif