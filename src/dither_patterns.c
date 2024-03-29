#pragma bank 255

#include <gbdk/platform.h>

#include "dither_patterns.h"
#include "gbcamera.h"

const uint8_t dither_high_light_values[NUM_CONTRAST_VALUES][4] = {
    {0x80, 0x8F, 0xD0, 0xE6},
    {0x82, 0x90, 0xC8, 0xE3},
    {0x84, 0x90, 0xC0, 0xE0},
    {0x85, 0x91, 0xB8, 0xDD},
    {0x86, 0x91, 0xB1, 0xDB},
    {0x87, 0x92, 0xAA, 0xD8},
    {0x88, 0x92, 0xA5, 0xD5},
    {0x89, 0x92, 0xA2, 0xD2},
    {0x8A, 0x92, 0xA1, 0xC8},
    {0x8B, 0x92, 0xA0, 0xBE},
    {0x8C, 0x92, 0x9E, 0xB4},
    {0x8D, 0x92, 0x9C, 0xAC},
    {0x8E, 0x92, 0x9B, 0xA5},
    {0x8F, 0x92, 0x99, 0xA0},
    {0x90, 0x92, 0x97, 0x9A},
    {0x92, 0x92, 0x92, 0x92},
};
const uint8_t dither_low_light_values[NUM_CONTRAST_VALUES][4] = {
    {0x80, 0x94, 0xDC, 0xFF},
    {0x82, 0x95, 0xD2, 0xFF},
    {0x84, 0x96, 0xCA, 0xFF},
    {0x86, 0x96, 0xC4, 0xFF},
    {0x88, 0x97, 0xBE, 0xFF},
    {0x8A, 0x97, 0xB8, 0xFF},
    {0x8B, 0x98, 0xB2, 0xF5},
    {0x8C, 0x98, 0xAC, 0xEB},
    {0x8D, 0x98, 0xAA, 0xDD},
    {0x8E, 0x98, 0xA8, 0xD0},
    {0x8F, 0x98, 0xA6, 0xC4},
    {0x90, 0x98, 0xA4, 0xBA},
    {0x92, 0x98, 0xA1, 0xB2},
    {0x94, 0x98, 0x9D, 0xA8},
    {0x96, 0x98, 0x99, 0xA0},
    {0x98, 0x98, 0x98, 0x98},
};
uint8_t dither_base_values[NUM_INTERVALS][NUM_BASE_VALUES];     // 3 ranges, 16 values each

const uint8_t pattern_null[PATTERN_MATRIX_SIZE] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

// original (standard)
const uint8_t pattern_standard[PATTERN_MATRIX_SIZE] = {
    0x00, 0x0C, 0x03, 0x0F,
    0x08, 0x04, 0x0B, 0x07,
    0x02, 0x0E, 0x01, 0x0D,
    0x0A, 0x06, 0x09, 0x05,
};

// 2x2
const uint8_t pattern_2x2[PATTERN_MATRIX_SIZE] = {
    0x01, 0x01, 0x0A, 0x0A,
    0x01, 0x01, 0x0A, 0x0A,
    0x0D, 0x0D, 0x03, 0x03,
    0x0D, 0x0D, 0x03, 0x03,
};

// Grid
const uint8_t pattern_grid[PATTERN_MATRIX_SIZE] = {
    0x0C, 0x08, 0x07, 0x0C,
    0x07, 0x01, 0x02, 0x07,
    0x07, 0x07, 0x02, 0x08,
    0x0D, 0x08, 0x08, 0x0C,
};

// Maze
const uint8_t pattern_maze[PATTERN_MATRIX_SIZE] = {
    0x00, 0x01, 0x03, 0x05,
    0x02, 0x0A, 0x0B, 0x0D,
    0x04, 0x0C, 0x07, 0x08,
    0x06, 0x0E, 0x09, 0x0F,
};

// Nest
const uint8_t pattern_nest[PATTERN_MATRIX_SIZE] = {
    0x00, 0x01, 0x08, 0x0B,
    0x02, 0x06, 0x0A, 0x0C,
    0x09, 0x0E, 0x03, 0x04,
    0x0D, 0x0F, 0x05, 0x07
};

// Fuzz
const uint8_t pattern_fuzz[PATTERN_MATRIX_SIZE] = {
    0x00, 0x09, 0x0E, 0x07,
    0x04, 0x0D, 0x02, 0x0B,
    0x08, 0x01, 0x06, 0x0F,
    0x0C, 0x05, 0x0A, 0x03
};

// Vertical
const uint8_t pattern_vertical[PATTERN_MATRIX_SIZE] = {
    0x00, 0x0A, 0x07, 0x0D,
    0x01, 0x0B, 0x04, 0x0E,
    0x02, 0x08, 0x05, 0x0F,
    0x03, 0x09, 0x06, 0x0C
};

// Horizontal
const uint8_t pattern_horizontal[PATTERN_MATRIX_SIZE] = {
    0x00, 0x01, 0x02, 0x03,
    0x0A, 0x0B, 0x08, 0x09,
    0x07, 0x04, 0x05, 0x06,
    0x0D, 0x0E, 0x0F, 0x0C
};

// Diagonal
const uint8_t pattern_diagonal[PATTERN_MATRIX_SIZE] = {
    0x00, 0x08, 0x04, 0x0C,
    0x0D, 0x01, 0x09, 0x05,
    0x06, 0x0E, 0x02, 0x0A,
    0x0B, 0x07, 0x0F, 0x03
};

const uint8_t * const * dithering_patterns[N_DITHER_TYPES][NUM_INTERVALS] = {
    [dither_type_Off]        = {pattern_null,       pattern_null,       pattern_null},
    [dither_type_Default]    = {pattern_standard,   pattern_standard,   pattern_standard},
    [dither_type_2x2]        = {pattern_2x2,        pattern_2x2,        pattern_2x2},
    [dither_type_Grid]       = {pattern_grid,       pattern_grid,       pattern_grid},
    [dither_type_Maze]       = {pattern_maze,       pattern_maze,       pattern_maze},
    [dither_type_Nest]       = {pattern_nest,       pattern_nest,       pattern_nest},
    [dither_type_Fuzz]       = {pattern_fuzz,       pattern_fuzz,       pattern_fuzz},
    [dither_type_Vertical]   = {pattern_vertical,   pattern_vertical,   pattern_vertical},
    [dither_type_Horizonral] = {pattern_horizontal, pattern_horizontal, pattern_horizontal},
    [dither_type_Mix]        = {pattern_horizontal, pattern_diagonal,   pattern_vertical}
};

void dither_gen_base_values(uint8_t a, uint8_t b, uint8_t * buffer) {
    uint16_t start = a << 8;
    uint16_t step = (a < b) ? ((uint16_t)(b << 8) - start) >> 4 : 0;
    for (uint8_t i = NUM_BASE_VALUES; i != 0; i--, start += step)
        * buffer++ = start >> 8;
}

void dither_pattern_apply(uint8_t dithering, uint8_t ditheringHighLight, uint8_t current_contrast) BANKED {
    const uint8_t * range = (ditheringHighLight) ? dither_high_light_values[current_contrast] : dither_low_light_values[current_contrast];
    dither_gen_base_values(*range, *(range + 1), dither_base_values[0]);
    range++;
    dither_gen_base_values(*range, *(range + 1), dither_base_values[1]);
    range++;
    dither_gen_base_values(*range, *(range + 1), dither_base_values[2]);

    uint8_t * dest = CAM_DITHERPATTERN;
    for (uint8_t i = 0; i != PATTERN_MATRIX_SIZE; i++) {
        *dest++ = dither_base_values[0][((const uint8_t *)(dithering_patterns[dithering][0]))[i]];
        *dest++ = dither_base_values[1][((const uint8_t *)(dithering_patterns[dithering][1]))[i]];
        *dest++ = dither_base_values[2][((const uint8_t *)(dithering_patterns[dithering][2]))[i]];
    }
}
