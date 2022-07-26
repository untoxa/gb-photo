#ifndef __SCREEN_H_INCLIDE__
#define __SCREEN_H_INCLIDE__

#include <gbdk/platform.h>
#include <stdint.h>

#include "vwf.h"

#define TO_TILE_ADDRESS(BASE, NO) ((BASE) + ((NO) << 4))

extern const uint8_t * const screen_tile_addresses[18];
extern const uint8_t screen_tile_map[360];

inline uint8_t screen_clear_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    return (w) ? fill_bkg_rect(x, y, w, h, color), w : w;
}

inline uint8_t screen_restore_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    return (w) ? set_bkg_submap(x, y, w, h, screen_tile_map, 20), w : w;
}

void screen_load_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t * picture);
void screen_load_thumbnail(uint8_t x, uint8_t y, uint8_t * picture);

inline uint8_t screen_text_render(uint8_t x, uint8_t y, const uint8_t * text) {
    return vwf_draw_text(screen_tile_addresses[y] + (x << 4), text);
}

inline uint8_t screen_text_out(uint8_t x, uint8_t y, const uint8_t * text) {
    return screen_restore_rect(x, y, vwf_draw_text(screen_tile_addresses[y] + (x << 4), text), 1);
}

#endif