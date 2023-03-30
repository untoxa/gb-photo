#ifndef __SCREEN_H_INCLIDE__
#define __SCREEN_H_INCLIDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "vwf.h"
#include "misc_assets.h"

#define TO_TILE_ADDRESS(BASE, NO) ((BASE) + ((NO) << 4))

BANKREF_EXTERN(module_screen)
BANKREF_EXTERN(module_display_off)
BANKREF_EXTERN(module_display_on)

extern const uint8_t * const screen_tile_addresses[18];
extern const uint8_t screen_tile_map[360];

inline uint8_t screen_clear_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    return (w) ? fill_bkg_rect(x, y, w, h, SLD_WHITE + BG_COLOR(color)), w : w;
}

inline uint8_t screen_restore_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    return (w) ? set_bkg_submap(x, y, w, h, screen_tile_map, 20), w : w;
}

#define IMAGE_NORMAL  false
#define IMAGE_FLIPPED true

void screen_load_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t * picture, bool flip);
void screen_load_image_banked(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t * picture, uint8_t bank, bool flip);
void screen_load_tile_banked(uint8_t x, uint8_t y, uint8_t * tile, uint8_t bank);

void screen_load_thumbnail(uint8_t x, uint8_t y, uint8_t * picture, uint8_t fill);
void screen_load_thumbnail_banked(uint8_t x, uint8_t y, uint8_t * picture, uint8_t fill, uint8_t bank);

inline uint8_t screen_text_render(uint8_t x, uint8_t y, const uint8_t * text) {
    return vwf_draw_text(screen_tile_addresses[y] + (x << 4), text);
}

inline uint8_t screen_text_out(uint8_t x, uint8_t y, const uint8_t * text) {
    return screen_restore_rect(x, y, vwf_draw_text(screen_tile_addresses[y] + (x << 4), text), 1);
}

uint8_t INIT_module_screen() BANKED;
uint8_t INIT_module_display_off() BANKED;
uint8_t INIT_module_display_on() BANKED;

#endif