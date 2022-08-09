#ifndef __MISC_ASSETS_H_INCLUDE__
#define __MISC_ASSETS_H_INCLUDE__

#define SOLID_WHITE         0xfeu
#define SOLID_BLACK         0xffu

#define CORNER_UL           0xfau
#define CORNER_UR           0xfbu
#define CORNER_DR           0xfdu
#define CORNER_DL           0xfcu

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

void misc_assets_init() BANKED;

#endif
