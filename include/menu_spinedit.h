#ifndef __MENU_SPINEDIT_H_INCLUDE__
#define __MENU_SPINEDIT_H_INCLUDE__

#include "menus.h"

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
} spinedit_params_t;

extern const menu_t SpinEditMenu;

#endif