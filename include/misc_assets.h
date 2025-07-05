#ifndef __MISC_ASSETS_H_INCLUDE__
#define __MISC_ASSETS_H_INCLUDE__

#include <gbdk/platform.h>

#include "compat.h"

#if defined(MASTERSYSTEM)
#define COLOR_BITS 4
#else
#define COLOR_BITS 2
#endif

#define BLACK_ON_WHITE      ((DMG_BLACK << COLOR_BITS) | DMG_WHITE)
#define WHITE_ON_BLACK      ((DMG_WHITE << COLOR_BITS) | DMG_BLACK)

#define DK_GR_ON_WHITE      ((DMG_DARK_GRAY << COLOR_BITS) | DMG_WHITE)
#define WHITE_ON_DK_GR      ((DMG_WHITE << COLOR_BITS) | DMG_DARK_GRAY)

#define LT_GR_ON_BLACK      ((DMG_LITE_GRAY << COLOR_BITS) | DMG_BLACK)
#define BLACK_ON_LT_GR      ((DMG_BLACK << COLOR_BITS) | DMG_LITE_GRAY)

#define DK_GR_ON_BLACK      ((DMG_DARK_GRAY << COLOR_BITS) | DMG_BLACK)
#define LT_GR_ON_WHITE      ((DMG_LITE_GRAY << COLOR_BITS) | DMG_WHITE)

#define HELP_CONTEXT_COLOR  LT_GR_ON_BLACK

#define BG_COLOR(c)         (c & ((1 << COLOR_BITS) - 1))
#define FG_COLOR(c)         ((c >> COLOR_BITS) & ((1 << COLOR_BITS) - 1))


#define SLD_WHITE           0xfcu
#define SLD_LITE_GRAY       0xfdu
#define SLD_DARK_GRAY       0xfeu
#define SLD_BLACK           0xffu

#define CORNER_UL           0xf8u
#define CORNER_UR           0xf9u
#define CORNER_DL           0xfau
#define CORNER_DR           0xfbu

#define ICON_BRIGHTNESS     "\x05"

#define ICON_SPIN_UP        "\x06"
#define ICON_SPIN_DOWN      "\x07"
#define ICON_MULTIPLE       "\x08"
#define ICON_CLOCK          "\x0b"

// profress bar
#define ICON_PROG_START     '\x0c'
#define ICON_PROG_FULL      '\x0d'
#define ICON_PROG_EMPTY     '\x0e'
#define ICON_PROG_END       '\x0f'

// REC indicator
#define ICON_REC            "\x10\x11\x12"

// menu icons
#define ICON_VOLTAGE        "\x13"
#define ICON_EDGE           "\x14"
#define ICON_GAIN           "\x15"
#define ICON_DITHER         "\x16"
#define ICON_CONTRAST       "\x17"
#define ICON_CBX            "\x18"
#define ICON_CBX_CHECKED    "\x19"

// game boy buttons
#define ICON_A              "\x1a"
#define ICON_B              "\x1b"
#define ICON_SELECT         "\x1c\x1d"
#define ICON_START          "\x1e\x1f"

// autoexp areas
#define ICON_AUTOEXP_TOP    "\x02\x01"
#define ICON_AUTOEXP_RIGHT  "\x02\x02"
#define ICON_AUTOEXP_BOTTOM "\x02\x03"
#define ICON_AUTOEXP_LEFT   "\x02\x04"

BANKREF_EXTERN(module_misc_assets)

inline void misc_render_progressbar(uint8_t value, uint8_t size, uint8_t * buffer) {
    // printer progress callback handler
    uint8_t * ptr = buffer;
    *ptr++ = ' ', *ptr++ = ICON_PROG_START;
    for (uint8_t i = 0; i != size; i++) {
        *ptr++ = (i > value) ? ICON_PROG_EMPTY : ICON_PROG_FULL;
    }
    *ptr++ = ICON_PROG_END;
    *ptr = 0;
}

uint8_t INIT_module_misc_assets(void) BANKED;

#endif
