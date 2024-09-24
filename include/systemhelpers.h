#ifndef __SYSTEMHELPERS_H_INCLUDE__
#define __SYSTEMHELPERS_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#include "systemdetect.h"

#define SPRITE_X_OFFSET (DEVICE_SPRITE_PX_OFFSET_X - 8)
#define SPRITE_Y_OFFSET (DEVICE_SPRITE_PX_OFFSET_Y - 16)

#define PALETTE_COUNT(A) (sizeof(A) / (sizeof(palette_color_t) * 4))

#define BKG_TILE_WIDTH(A) (A ## _WIDTH / 8)
#define BKG_TILE_HEIGHT(A) (A ## _HEIGHT / A ## _TILE_H)
#define BKG_MAP(A) (A ## _map)
#define BKG_ATTR(A) (A ## _map_attributes)
#define BKG_TILES(A) (A ## _tiles)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CONSTRAINT(v, min, max) MAX(MIN((v), (max)), (min))

#define TO_SUBPIXEL(a) ((a) << 4u)
#define TO_PIXEL(a) ((a) >> 4u)
#define TO_TILE(a) ((a) >> 7u)
#define PIXEL_TO_TILE(a) ((a) >> 3u)
#define TILE_TO_PIXEL(a) ((a) << 3u)
#define TILE_TO_SUBPIXEL(a) ((a) << 7u)

#define CURSOR_MAX_SPEED 48
#define CURSOR_ACCELERATE 1
#define CURSOR_DECELERATE 2

#define MOVE_BKG(x, y) (shadow_scx_reg = (x), shadow_scy_reg = WRAP_SCROLL_Y((y)))

#define LENGTH(a) (sizeof((a))/sizeof((a)[0]))
#define MAX_INDEX(a) ((sizeof((a))/sizeof((a)[0]))-1)
#define LAST_ITEM(a) (a+MAX_INDEX(a))

#define __ADD_CHECK__(a,b) __ADD_CHECK___(a,b)
#define __ADD_CHECK___(a,b) __CHECK_##a##_##b
#define CHECK_SIZE(typ,sz) typedef char __ADD_CHECK__(typ,__LINE__)[ (sizeof(typ) == (sz)) ? 1 : "error"]
#define CHECK_SIZE_NOT_LARGER(typ,sz) typedef char __ADD_CHECK__(typ,__LINE__)[ (sizeof(typ) <= (sz)) ? 1 : "error"]

#if defined(SEGA)
    #define WRAP_SCROLL_Y(y) ((y) % 224u)
#elif defined(NINTENDO)
    #define WRAP_SCROLL_Y(y) y
#endif

inline void fill_bkg_rect_ex(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile, uint8_t attr) {
    if (_is_COLOR) {
        VBK_REG = 1;
        fill_bkg_rect(x, y, w, h, attr);
        VBK_REG = 0;
    }
    fill_bkg_rect(x, y, w, h, tile);
}

inline void set_bkg_tiles_ex(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * map, const uint8_t * attr) {
    if (_is_COLOR) {
        VBK_REG = 1;
        set_bkg_tiles(x, y, w, h, (uint8_t *)attr);
        VBK_REG = 0;
    }
    set_bkg_tiles(x, y, w, h, (uint8_t *)map);
}

inline void set_bkg_based_tiles_ex(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * map, const uint8_t * attr, uint8_t base_tile, uint8_t base_palette) {
    if (_is_COLOR) {
        VBK_REG = 1;
        set_bkg_based_tiles(x, y, w, h, (uint8_t *)attr, base_palette);
        VBK_REG = 0;
    }
    set_bkg_based_tiles(x, y, w, h, (uint8_t *)map, base_tile);
}

inline uint16_t swap_bytes(uint16_t value) {
    return ((value >> 8) & 0xffu)| (value << 8);
}

#endif