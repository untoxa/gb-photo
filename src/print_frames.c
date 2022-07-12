#include "print_frames.h"

#include "wild_frame.h"

BANKREF(print_frames)

const frame_desc_t print_frames[] = {
    {
        .height = 14,
        .map = NULL,
        .map_bank = 0,
        .tiles = NULL,
        .tiles_bank = 0,
        .image_y = 0, 
        .image_x = 2
    },{
        .height = wild_frame_HEIGHT / wild_frame_TILE_H,
        .map = wild_frame_map,
        .map_bank = BANK(wild_frame),
        .tiles = wild_frame_tiles,
        .tiles_bank = BANK(wild_frame),
        .image_y = 14, 
        .image_x = 2
    }
};