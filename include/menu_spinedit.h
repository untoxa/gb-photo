#ifndef __MENU_SPINEDIT_H_INCLUDE__
#define __MENU_SPINEDIT_H_INCLUDE__

#include "menus.h"

typedef struct spinedit_params_t spinedit_params_t;

typedef uint8_t change_handler_t (const struct menu_t * menu, const struct menu_item_t * selection, spinedit_params_t * param);

typedef struct spinedit_value_names_t {
    const struct spinedit_value_names_t * next;
    uint8_t value;
    uint8_t * name;
} spinedit_value_names_t;

typedef struct spinedit_params_t {
    const uint8_t * caption;
    uint8_t min_value;
    uint8_t max_value;
    uint8_t * value;
    const spinedit_value_names_t * names;
    change_handler_t * onChange;
} spinedit_params_t;

extern const menu_t SpinEditMenu;

#endif