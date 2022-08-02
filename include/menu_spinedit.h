#ifndef __MENU_SPINEDIT_H_INCLUDE__
#define __MENU_SPINEDIT_H_INCLUDE__

#include "menus.h"

typedef struct spinedit_params_t {
    const uint8_t * caption;
    uint8_t min_value;
    uint8_t max_value;
    uint8_t * value;
} spinedit_params_t;

extern const menu_t SpinEditMenu;

#endif