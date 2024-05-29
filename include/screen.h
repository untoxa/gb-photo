#ifndef __SCREEN_H_INCLIDE__
#define __SCREEN_H_INCLIDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "vwf.h"
#include "misc_assets.h"

#define TO_TILE_ADDRESS(BASE, NO) ((BASE) + ((NO) << DEVICE_TILE_SIZE_BITS))

#if defined(NINTENDO)
#define TILE_BANK_0 _VRAM8800
#define TILE_BANK_1 _VRAM8000
#elif defined(SEGA)
static uint8_t AT(0x4000) TILE_BANK_0[];
static uint8_t AT(0x6000) TILE_BANK_1[];
#endif


BANKREF_EXTERN(module_screen)
BANKREF_EXTERN(module_display_off)
BANKREF_EXTERN(module_display_on)

extern const uint8_t * const screen_tile_addresses[DEVICE_SCREEN_HEIGHT];
extern const uint8_t screen_tile_map[DEVICE_SCREEN_HEIGHT * DEVICE_SCREEN_WIDTH];
#if defined(SEGA)
extern const uint8_t screen_tile_attr[DEVICE_SCREEN_HEIGHT * DEVICE_SCREEN_WIDTH];
#endif

inline uint8_t screen_clear_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    return (w) ? fill_bkg_rect(x, y, w, h, SLD_WHITE + BG_COLOR(color)), w : w;
}

inline uint8_t screen_restore_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
#if defined(NINTENDO)
    return (w) ? set_bkg_submap(x, y, w, h, screen_tile_map, DEVICE_SCREEN_WIDTH), w : w;
#elif defined(SEGA)
    if (w) {
        set_bkg_submap(x, y, w, h, screen_tile_map, DEVICE_SCREEN_WIDTH);
        set_bkg_submap_attributes(x, y, w, h, screen_tile_attr, DEVICE_SCREEN_WIDTH);
    }
    return w;
#endif
}

#define IMAGE_NORMAL  false
#define IMAGE_FLIPPED true

void screen_transfer_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture);
void screen_load_image_flipped(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture);
void screen_load_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture);

inline void screen_load_live_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture, bool flip, bool DMA) {
    if (!flip) {
        if (DMA) screen_transfer_image(x, y, w, h, picture); else screen_load_image(x, y, w, h, picture);
    } else screen_load_image_flipped(x, y, w, h, picture);
}

void screen_load_image_banked(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture, uint8_t bank);
void screen_load_tile_banked(uint8_t x, uint8_t y, uint8_t * tile, uint8_t bank);

void screen_load_thumbnail(uint8_t x, uint8_t y, uint8_t * picture, uint8_t fill);
void screen_load_thumbnail_banked(uint8_t x, uint8_t y, uint8_t * picture, uint8_t fill, uint8_t bank);

inline uint8_t screen_text_render(uint8_t x, uint8_t y, const uint8_t * text) {
    return vwf_draw_text(screen_tile_addresses[y] + (x << DEVICE_TILE_SIZE_BITS), text, 0);
}

inline uint8_t screen_text_out(uint8_t x, uint8_t y, const uint8_t * text) {
    return screen_restore_rect(x, y, vwf_draw_text(screen_tile_addresses[y] + (x << DEVICE_TILE_SIZE_BITS), text, 0), 1);
}

uint8_t INIT_module_screen(void) BANKED;
uint8_t INIT_module_display_off(void) BANKED;
uint8_t INIT_module_display_on(void) BANKED;

#endif