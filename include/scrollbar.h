#ifndef __SCROLLBAR_H_INCLUDE__
#define __SCROLLBAR_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct scrollbar_t {
    struct scrollbar_t * prev;
    uint8_t x, y, len;
    bool vertical;
    uint8_t position;
} scrollbar_t;

extern scrollbar_t * scrollbars;

inline scrollbar_t * scrollbar_add(scrollbar_t * ss, uint8_t x, uint8_t y, uint8_t len, bool vertical) {
    ss->prev = scrollbars, scrollbars = ss;
    ss->x = x, ss->y = y, ss->len = len, ss->vertical = vertical;
    return ss;
}

inline void scrollbar_destroy_all() {
    scrollbars = NULL;
}

uint8_t scrollbar_render_all(uint8_t hiwater);
void scrollbar_repaint_all() BANKED;
void scrollbar_set_position(scrollbar_t * ss, uint16_t value, uint16_t min, uint16_t max) BANKED;

#endif