#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "gbcamera.h"
#include "systemhelpers.h"
#include "state_camera.h"

#define XY2PTR(X,Y) (last_seen + ((Y) * 16 * 16) + (X * 16))

static const uint8_t * const histogram_points_center[] = {
        XY2PTR( 6,  4), XY2PTR( 9,  4),
    XY2PTR( 5,  5),         XY2PTR(10,  5),
        XY2PTR( 7,  6), XY2PTR( 8,  6),
        XY2PTR( 7,  7), XY2PTR( 8,  7),
    XY2PTR( 5,  8),         XY2PTR(10,  8),
        XY2PTR( 6,  9), XY2PTR( 9,  9)
};

static const uint8_t * const histogram_points_top[LENGTH(histogram_points_center)] = {
        XY2PTR( 6,  0), XY2PTR( 9,  0),
    XY2PTR( 5,  1),         XY2PTR(10,  1),
        XY2PTR( 7,  2), XY2PTR( 8,  2),
        XY2PTR( 7,  3), XY2PTR( 8,  3),
    XY2PTR( 5,  4),         XY2PTR(10,  4),
        XY2PTR( 6,  5), XY2PTR( 9,  5)
};
static const uint8_t * const histogram_points_right[LENGTH(histogram_points_center)] = {
        XY2PTR(11,  4), XY2PTR(14,  4),
    XY2PTR(10,  5),         XY2PTR(15,  5),
        XY2PTR(12,  6), XY2PTR(13,  6),
        XY2PTR(12,  7), XY2PTR(13,  7),
    XY2PTR(10,  8),         XY2PTR(15,  8),
        XY2PTR(11,  9), XY2PTR(14,  9)
};
static const uint8_t * const histogram_points_bottom[LENGTH(histogram_points_center)] = {
        XY2PTR( 6,  8), XY2PTR( 9,  8),
    XY2PTR( 5,  9),         XY2PTR(10,  9),
        XY2PTR( 7, 10), XY2PTR( 8,  10),
        XY2PTR( 7, 11), XY2PTR( 8,  11),
    XY2PTR( 5, 12),         XY2PTR(10, 12),
        XY2PTR( 6, 13), XY2PTR( 9, 13)
};
static const uint8_t * const histogram_points_left[LENGTH(histogram_points_center)] = {
        XY2PTR( 1,  4), XY2PTR( 4,  4),
    XY2PTR( 0,  5),         XY2PTR(5,  5),
        XY2PTR( 2,  6), XY2PTR( 3,  6),
        XY2PTR( 2,  7), XY2PTR( 3,  7),
    XY2PTR( 0,  8),         XY2PTR( 5,  8),
        XY2PTR( 1,  9), XY2PTR( 4,  9)
};

static const uint8_t ** const histogram_areas[N_AUTOEXP_AREAS] = {
    histogram_points_center, histogram_points_top, histogram_points_right, histogram_points_bottom, histogram_points_left
};

void AT(LENGTH(histogram_points_center) * 64 * 1.5) __histogram_target_value;
void AT(LENGTH(histogram_points_center)) __histogram_points_count;
void AT(LENGTH(histogram_points_center) * 64 * 3) __histogram_max_value;


extern const uint8_t bit_count_table[];     // aligned to 256 byte boundary

static uint8_t histogram_counter;

uint16_t calculate_tile(uint8_t * data) NAKED {
    data;
#ifdef NINTENDO
    __asm
        ld h, d
        ld l, e
        ld de, #_bit_count_table
        ld bc, #0

        .rept 8
            ld a, (hl+)
            ld e, a

            ld a, (de)          ; add with weight 1
            add c
            ld c, a
            adc b
            sub c
            ld b, a

            ld a, (hl+)
            ld e, a

            ld a, (de)          ; add with weight 2
            add a
            add c
            ld c, a
            adc b
            sub c
            ld b, a
        .endm
        ret
    __endasm;
#else
    __asm
        ld de, #0
        ret
    __endasm;
#endif
}

int16_t calculate_histogram(autoexp_area_e area) BANKED {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    static uint16_t histogram;
    histogram = 0;
    for (uint8_t i = LENGTH(histogram_points_center), * const * ptr = histogram_areas[area]; i != 0; i--) histogram += calculate_tile(*ptr++);
    return histogram;
}