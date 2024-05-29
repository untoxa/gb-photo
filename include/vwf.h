#ifndef _VWF_H_INCLUDE
#define _VWF_H_INCLUDE

#include <gbdk/platform.h>

#include <stdint.h>

#if defined(NINTENDO)
#define DEVICE_TILE_SIZE_BITS 4
#elif defined(SEGA)
#define DEVICE_TILE_SIZE_BITS 5
#endif

#define __VWF_BANK_PREFIX(A) __bank_##A
#define TO_VWF_FARPTR(A) {.bank = (char)&(__VWF_BANK_PREFIX(A)), .ptr = (void *)&(A)}

#define RECODE_7BIT 1
#define FONT_VWF 2

typedef struct vwf_farptr_t {
    UINT8 bank;
    void * ptr;
} vwf_farptr_t;

typedef struct font_desc_t {
    uint8_t attr;
    const uint8_t * recode_table;
    const uint8_t * widths;
    const uint8_t * bitmaps;
} font_desc_t;

extern vwf_farptr_t vwf_fonts[4];
extern uint8_t vwf_mode;
extern uint8_t vwf_tab_size;

void vwf_load_font(uint8_t idx, const void * font, uint8_t bank);
void vwf_activate_font(uint8_t idx);
uint8_t vwf_text_width(const unsigned char * str);
uint8_t vwf_draw_text(const uint8_t * base_tile, const unsigned char * str, uint8_t offset);
uint8_t * vwf_next_tile(void);

inline void vwf_set_colors(uint8_t fgcolor, uint8_t bgcolor) {
    set_1bpp_colors(fgcolor, bgcolor);
}

inline void vwf_set_tab_size(uint8_t tab_size) {
    vwf_tab_size = (tab_size) ? tab_size : 1;
}

#endif