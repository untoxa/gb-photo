#ifndef __PRINT_FRAMES_H_INCLUDE__
#define __PRINT_FRAMES_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

typedef struct frame_desc_t {
    uint8_t height;                 // height of the printed image with the frame
    const uint8_t * map;            // tilemap of the frame
    uint8_t map_bank;               // bank of the tilemap
    const uint8_t * tiles;          // tiledata used by the frame
    uint8_t tiles_bank;             // bank of the tiledata
    uint8_t image_y, image_x;       // y and x coordinates in tiles of the upper left corner of the photo
    const uint8_t * caption;        // frame title
    const uint8_t * desc;           // frame description
} frame_desc_t;

BANKREF_EXTERN(print_frames)

extern const frame_desc_t print_frames[];

#endif