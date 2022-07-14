#ifndef __MENUS_H_INCLUDE__
#define __MENUS_H_INCLUDE__

#include <stdint.h>

typedef struct menu_t;
typedef struct menu_item_t;

typedef uint8_t menu_handler_t (const struct menu_t * self, uint8_t * param);
typedef uint8_t idle_handler_t (const struct menu_t * menu, const struct menu_item_t * selection);
typedef uint8_t menu_translate_t (const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
typedef uint8_t * item_handler_t (const struct menu_t * menu, const struct menu_item_t * self);

typedef struct menu_item_t {
    struct menu_item_t * prev;
    struct menu_item_t * next;
    struct menu_t * sub;
    uint8_t * sub_params;
    uint8_t ofs_x;
    uint8_t ofs_y;
    uint8_t width;
    const uint8_t * caption;
    const uint8_t * helpcontext;
    item_handler_t * onPaint;
    uint8_t result;
} menu_item_t;

typedef struct menu_t {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t cancel_mask;
    uint8_t cancel_result; 
    menu_item_t * items;
    menu_handler_t * onShow;
    idle_handler_t * onIdle;
    idle_handler_t * onHelpContext;
    menu_translate_t * onTranslateKey;
    menu_translate_t * onTranslateSubResult;
} menu_t;

void menu_text_out(uint8_t x, uint8_t y, uint8_t w, uint8_t c, const uint8_t * text);
const menu_item_t * menu_move_selection(const menu_t * menu, const menu_item_t * selection, const menu_item_t * new_selection);

uint8_t menu_execute(const menu_t * menu, uint8_t * param);

#endif