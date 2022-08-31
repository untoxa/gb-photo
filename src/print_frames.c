#include "print_frames.h"

#include "frame_photo.h"
#include "frame_wild.h"

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
        .height = frame_photo_HEIGHT / frame_photo_TILE_H,
        .map = frame_photo_map,
        .map_bank = BANK(frame_photo),
        .tiles = frame_photo_tiles,
        .tiles_bank = BANK(frame_photo),
        .image_y = 2,
        .image_x = 2,
        .caption = "Photo! frame",
        .desc = "\"Photo!\" frame",
    },{
        .height = frame_wild_HEIGHT / frame_wild_TILE_H,
        .map = frame_wild_map,
        .map_bank = BANK(frame_wild),
        .tiles = frame_wild_tiles,
        .tiles_bank = BANK(frame_wild),
        .image_y = 14,
        .image_x = 2,
        .caption = "GB Camera",
        .desc = "\"GB Camera\" frame",
    }
};