#pragma bank 255

#include <stdint.h>
#include <string.h>
#include "systemhelpers.h"
#include "screen.h"

#include "print_frames.h"

#include "frame_photo.h"
#include "frame_photo_thumb.h"
#include "frame_lens.h"
#include "frame_lens_thumb.h"
#include "frame_trinitron.h"
#include "frame_trinitron_thumb.h"
#include "frame_sgb.h"
#include "frame_sgb_thumb.h"
#include "frame_slide.h"
#include "frame_slide_thumb.h"
#include "frame_tiles.h"
#include "frame_tiles_thumb.h"
#include "frame_pcb.h"
#include "frame_pcb_thumb.h"
#include "frame_zebra.h"
#include "frame_zebra_thumb.h"

#include "frame_wild.h"

BANKREF(print_frames)

#define IMG_HEIGHT(A) (A ## _HEIGHT / A ## _TILE_H)

const frame_desc_t print_frames[] = {
    {
        .height = 14,
        .map = NULL,
        .map_bank = 0,
        .tiles = NULL,
        .tiles_bank = 0,
        .thumb = NULL,
        .thumb_bank = 0,
        .image_y = 0,
        .image_x = 2,
        .caption = "No Frame",
        .desc = "Print without frame"
    }, {
        .height = IMG_HEIGHT(frame_photo),
        .map = frame_photo_map,
        .map_bank = BANK(frame_photo),
        .tiles = frame_photo_tiles,
        .tiles_bank = BANK(frame_photo),
        .thumb = frame_photo_thumb_tiles,
        .thumb_bank = BANK(frame_photo_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "Photo!",
        .desc = "\"Photo!\" frame"
    }, {
        .height = IMG_HEIGHT(frame_lens),
        .map = frame_lens_map,
        .map_bank = BANK(frame_lens),
        .tiles = frame_lens_tiles,
        .tiles_bank = BANK(frame_lens),
        .thumb = frame_lens_thumb_tiles,
        .thumb_bank = BANK(frame_lens_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "Lens",
        .desc = "\"Lens\" frame"
    }, {
        .height = IMG_HEIGHT(frame_trinitron),
        .map = frame_trinitron_map,
        .map_bank = BANK(frame_trinitron),
        .tiles = frame_trinitron_tiles,
        .tiles_bank = BANK(frame_trinitron),
        .thumb = frame_trinitron_thumb_tiles,
        .thumb_bank = BANK(frame_trinitron_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "Trinitron",
        .desc = "\"Trinitron\" frame"
    }, {
        .height = IMG_HEIGHT(frame_sgb),
        .map = frame_sgb_map,
        .map_bank = BANK(frame_sgb),
        .tiles = frame_sgb_tiles,
        .tiles_bank = BANK(frame_sgb),
        .thumb = frame_sgb_thumb_tiles,
        .thumb_bank = BANK(frame_sgb_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "SGB",
        .desc = "\"Super GB\" frame"
    }, {
        .height = IMG_HEIGHT(frame_slide),
        .map = frame_slide_map,
        .map_bank = BANK(frame_slide),
        .tiles = frame_slide_tiles,
        .tiles_bank = BANK(frame_slide),
        .thumb = frame_slide_thumb_tiles,
        .thumb_bank = BANK(frame_slide_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "Slide",
        .desc = "\"Slide\" frame"
    }, {
        .height = IMG_HEIGHT(frame_tiles),
        .map = frame_tiles_map,
        .map_bank = BANK(frame_tiles),
        .tiles = frame_tiles_tiles,
        .tiles_bank = BANK(frame_tiles),
        .thumb = frame_tiles_thumb_tiles,
        .thumb_bank = BANK(frame_tiles_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "Tiles",
        .desc = "\"Tiles\" frame"
    }, {
        .height = IMG_HEIGHT(frame_pcb),
        .map = frame_pcb_map,
        .map_bank = BANK(frame_pcb),
        .tiles = frame_pcb_tiles,
        .tiles_bank = BANK(frame_pcb),
        .thumb = frame_pcb_thumb_tiles,
        .thumb_bank = BANK(frame_pcb_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "PCB",
        .desc = "\"PCB\" frame"
    }, {
        .height = IMG_HEIGHT(frame_zebra),
        .map = frame_zebra_map,
        .map_bank = BANK(frame_zebra),
        .tiles = frame_zebra_tiles,
        .tiles_bank = BANK(frame_zebra),
        .thumb = frame_zebra_thumb_tiles,
        .thumb_bank = BANK(frame_zebra_thumb),
        .image_y = 2,
        .image_x = 2,
        .caption = "Zebra",
        .desc = "\"Zebra\" frame"
    }, {
        .height = IMG_HEIGHT(frame_wild),
        .map = frame_wild_map,
        .map_bank = BANK(frame_wild),
        .tiles = frame_wild_tiles,
        .tiles_bank = BANK(frame_wild),
        .thumb = NULL,
        .thumb_bank = 0,
        .image_y = 14,
        .image_x = 2,
        .caption = "GB Camera",
        .desc = "\"GB Camera\" frame"
    }
};

void frame_get_caption(uint8_t * caption, uint8_t frame) BANKED {
    if (frame < LENGTH(print_frames)) strcpy(caption, print_frames[frame].caption); else *caption = 0;
}
void frame_get_desc(uint8_t * desc, uint8_t frame) BANKED {
    if (frame < LENGTH(print_frames)) strcpy(desc, print_frames[frame].desc); else *desc = 0;
}
void frame_display_thumbnail(uint8_t x, uint8_t y, uint8_t frame, uint8_t color) BANKED {
    if ((frame < LENGTH(print_frames)) && (print_frames[frame].thumb)) {
        screen_load_image_banked(x, y, 4, 4, print_frames[frame].thumb, print_frames[frame].thumb_bank, false);
        screen_restore_rect(x, y, 4, 4);
    } else screen_clear_rect(x, y, 4, 4, color);
}