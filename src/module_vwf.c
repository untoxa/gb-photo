#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "vwf.h"
#include "module_vwf.h"

// graphic assets
#include "font_proportional.h"
#include "font_fancy.h"

BANKREF(module_vwf)

// initialize the VWF subsystem
uint8_t INIT_module_vwf() BANKED {
    vwf_load_font(0, font_proportional, BANK(font_proportional));
    vwf_load_font(1, font_fancy, BANK(font_fancy));
    vwf_activate_font(0);
#if !defined(NINTENDO)
    vwf_set_colors(2, 1);
#endif
    return 0;
}