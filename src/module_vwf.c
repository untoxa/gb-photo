#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "vwf.h"
#include "module_vwf.h"

// graphic assets
#include "font_main.h"
#include "font_about.h"

BANKREF(module_vwf)

// initialize the VWF subsystem
uint8_t INIT_module_vwf(void) BANKED {
    vwf_load_font(0, font_main, BANK(font_main));
    vwf_load_font(1, font_about, BANK(font_about));
    vwf_activate_font(0);
    vwf_set_colors(DMG_BLACK, DMG_WHITE);
    return 0;
}