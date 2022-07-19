#include "print_frames.h"

#include "pxlr_frame.h"
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
        .image_x = 2,
        .caption = "No Frame",
        .desc = "Print without frame"
    },{
        .height = pxlr_frame_HEIGHT / pxlr_frame_TILE_H,
        .map = pxlr_frame_map,
        .map_bank = BANK(pxlr_frame),
        .tiles = pxlr_frame_tiles,
        .tiles_bank = BANK(pxlr_frame),
        .image_y = 2,
        .image_x = 2,
        .caption = "PXLR Studio",
        .desc = "\"PXLR Studio\" frame",
    },{
        .height = wild_frame_HEIGHT / wild_frame_TILE_H,
        .map = wild_frame_map,
        .map_bank = BANK(wild_frame),
        .tiles = wild_frame_tiles,
        .tiles_bank = BANK(wild_frame),
        .image_y = 14,
        .image_x = 2,
        .caption = "GB Camera",
        .desc = "\"GB Camera\" frame",
    }
};