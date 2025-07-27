#pragma bank 255

#include "fade.h"
#include "palette.h"

#define PAL_RED(C)   (((C)      ) & 0x0Fu)
#define PAL_GREEN(C) (((C) >>  4) & 0x0Fu)
#define PAL_BLUE(C)  (((C) >>  8) & 0x0Fu)

#define WAIT_WRITABLE_CRAM while (VCOUNTER < 216) {}

static int8_t BR, BB, BG;

palette_color_t UpdateColor(uint8_t i, palette_color_t col) {
    if (i == 0) return col;

    if (i == 3) return SprPalette[0].c0;

    int8_t R = (PAL_RED(col) << 3), G = (PAL_GREEN(col) << 3), B = (PAL_BLUE(col) << 3);

    int8_t DR = (int8_t)(BR - R) >> 2, DG = (int8_t)(BG - G) >> 2, DB = (int8_t)(BB - B) >> 2;

    R += DR, G += DG, B += DB;
    if (i == 2) R += DR, G += DG, B += DB;

    return RGB(R >> 3, G >> 3, B >> 3);
}

void FadeStepColor(uint8_t i) {
    static palette_color_t palette[16];
    static palette_color_t palette_s[16];
    palette_color_t* col = (palette_color_t*)(&BkgPalette);
    palette_color_t* col_s = (palette_color_t*)(&SprPalette);

    for(uint8_t c = 0; c < 16; ++c, ++col, ++col_s) {
        palette[c] = UpdateColor(i, *col);
        palette_s[c] = UpdateColor(i, *col_s);
    };

    WAIT_WRITABLE_CRAM;	// avoid snow on screen

    set_bkg_palette(0, 1, palette);
    set_sprite_palette(0, 1, palette_s);

    DISPLAY_ON;
    vsync();
    vsync();
}

void fade_out_modal(void) BANKED {
    BR = (PAL_RED(SprPalette[0].c0) << 3), BG = (PAL_GREEN(SprPalette[0].c0) << 3), BB = (PAL_BLUE(SprPalette[0].c0) << 3);
    FadeStepColor(0);
    FadeStepColor(1);
    FadeStepColor(2);
    FadeStepColor(3);
    DISPLAY_OFF;
}

void fade_in_modal(void) BANKED {
    BR = (PAL_RED(SprPalette[0].c0) << 3), BG = (PAL_GREEN(SprPalette[0].c0) << 3), BB = (PAL_BLUE(SprPalette[0].c0) << 3);
    FadeStepColor(3);
    FadeStepColor(2);
    FadeStepColor(1);
    FadeStepColor(0);
}
