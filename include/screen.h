#ifndef __SCREEN_H_INCLIDE__
#define __SCREEN_H_INCLIDE__

#include <gbdk/platform.h>
#include <stdint.h>

#define IMAGE_DISPLAY_X 2
#define IMAGE_DISPLAY_Y 2

#define THUMBNAIL_DISPLAY_X 2
#define THUMBNAIL_DISPLAY_Y 1 
#define THUMBNAIL_DISPLAY_WIDTH 16
#define THUMBNAIL_DISPLAY_HEIGHT 16

#define MAX_PREVIEW_THUMBNAILS 16
#define MAX_PREVIEW_PAGES 2

#define SOLID_WHITE 0xfeu
#define SOLID_BLACK 0xffu

#define CORNER_UL 0xfau
#define CORNER_UR 0xfbu
#define CORNER_DR 0xfcu
#define CORNER_DL 0xfdu


extern uint8_t * const screen_tile_addresses[18];
extern const uint8_t screen_tile_map[360];

inline void screen_clear_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    fill_bkg_rect(x, y, w, h, color);
}

inline void screen_restore_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    set_bkg_submap(x, y, w, h, screen_tile_map, 20);
}

void screen_init();

uint8_t screen_show_picture(uint8_t n);
uint8_t screen_show_tumbnails(uint8_t start);
void screen_text_out(uint8_t x, uint8_t y, const uint8_t * text);

#endif