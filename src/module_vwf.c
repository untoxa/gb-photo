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
    vwf_set_colors(DMG_BLACK, DMG_WHITE);
    return 0;
}