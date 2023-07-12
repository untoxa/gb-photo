#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "gbcamera.h"
#include "systemhelpers.h"

#define HOTPOINT_AREA 0
#define USE_DIFFERENCE 0

#define XY2PTR(X,Y) (last_seen + ((Y) * 16 * 16) + (X * 16))
#define ABSDELTA(A, B) (((A)>(B))?((A)-(B)):((B)-(A)))

#if (HOTPOINT_AREA==0)
static const uint8_t * const histogram_points[] = {
        XY2PTR( 6,  4), XY2PTR( 9,  4),
    XY2PTR( 5,  5),         XY2PTR(10,  5),
        XY2PTR( 7,  6), XY2PTR( 8,  6),
        XY2PTR( 7,  7), XY2PTR( 8,  7),
    XY2PTR( 5,  8),         XY2PTR(10,  8),
        XY2PTR( 6,  9), XY2PTR( 9,  9)
};
#elif (HOTPOINT_AREA==1)
static const uint8_t * const histogram_points[] = {
                        XY2PTR( 5,  2),         XY2PTR(10,  2),
    XY2PTR(2, 3),                                                   XY2PTR(13,  3),
                            XY2PTR( 7,  4), XY2PTR( 8,  4),
                        XY2PTR( 5,  5),         XY2PTR(10,  5),
    XY2PTR(1, 6),           XY2PTR( 7,  6), XY2PTR( 8,  6),         XY2PTR(14,  6),
        XY2PTR(3, 7),       XY2PTR( 7,  7), XY2PTR( 8,  7),     XY2PTR(12,  7),
                        XY2PTR( 5,  8),         XY2PTR(10,  8),
                            XY2PTR( 7,  9), XY2PTR( 8,  9),
    XY2PTR(2, 10),                                                   XY2PTR(13, 10),
                        XY2PTR( 5, 11),         XY2PTR(10, 11)
};
#endif

#if (USE_DIFFERENCE==0)
void AT(LENGTH(histogram_points) * 64 * 1.5) __histogram_target_value;
#else
int16_t __histogram_target_value = 0;
#endif
void AT(LENGTH(histogram_points)) __histogram_points_count;
void AT(LENGTH(histogram_points) * 64 * 3) __histogram_max_value;


static uint8_t histogram_counter;
uint16_t histogram[4];

void calculate_tile(uint8_t * data) NAKED {
    data;
#ifdef NINTENDO
    __asm
.macro  .ADD_A_HL
        add l
        ld l, a
        adc h
        sub l
        ld h, a
.endm
.macro  .CALC ?lbl
        rlc b
        rla
        rlc c
        rla
        and #0x03
        rlca
        ld hl, #_histogram
        .ADD_A_HL
        inc (hl)
        jr nz, lbl
        inc hl
        inc (hl)
lbl:
.endm
        ld a, #8
        ld (_histogram_counter), a
1$:
        ld a, (de)
        ld c, a
        inc e
        ld a, (de)
        ld b, a
        inc e
        .rept 8
            .CALC
        .endm
        ld hl, #_histogram_counter
        dec (hl)
        jp nz, 1$
        ret
    __endasm;
#else
    __asm
        ld hl, #0
        ret
    __endasm;
#endif
}

int16_t calculate_histogram(void) BANKED {
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    *(uint64_t *)&histogram = 0;
    for (uint8_t i = LENGTH(histogram_points), * const * ptr = histogram_points; i != 0; i--) calculate_tile(*ptr++);
#if (USE_DIFFERENCE==0)
    return histogram[1] + (histogram[2] * 2) + (histogram[3] * 3);
#else
    return ABSDELTA(histogram[0], histogram[1]) + ABSDELTA(histogram[0], histogram[2]) + ABSDELTA(histogram[0], histogram[3]) +
                                                  ABSDELTA(histogram[1], histogram[2]) + ABSDELTA(histogram[1], histogram[3]) +
                                                                                         ABSDELTA(histogram[2], histogram[3]);
#endif
}