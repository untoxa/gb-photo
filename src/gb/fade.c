#pragma bank 255

#include "fade.h"
#include "palette.h"

#define PAL_RED(C)   (((C)      ) & 0x1Fu)
#define PAL_GREEN(C) (((C) >>  5) & 0x1Fu)
#define PAL_BLUE(C)  (((C) >> 10) & 0x1Fu)

inline int16_t DespRight(int16_t a, uint8_t b) {
	return a >> b;
}

inline uint8_t FadeInOp(uint8_t c, uint8_t i) {
	return (c < i) ? 0: (c - i);
}

static uint8_t * const pals[] = {&BGP_REG, &OBP0_REG, &OBP1_REG};

void FadeDMG(uint8_t fadein) {
	static uint8_t colors[12];
	uint8_t i, j;
	uint8_t* c = colors;
	uint8_t p;

	//Pick current palette colors
	for(i = 0; i != 3; ++i) {
		p = DMG_palette[i];
		for(j = 0; j != 8; j += 2, ++c) {
			*c = (DespRight(p, j)) & 0x03;
		}
	}

	for(i = 0; i != 4; ++i) {
		p = fadein ? 3 - i : i;
		for(j = 0; j != 3; ++j) {
			c = &colors[j << 2];
			*pals[j] = DMG_PALETTE(FadeInOp(c[0], p), FadeInOp(c[1], p), FadeInOp(c[2], p), FadeInOp(c[3], p));
		}
		vsync();
		vsync();
	}
}

palette_color_t UpdateColor(uint8_t i, uint16_t col) {
	return RGB(PAL_RED(col) | DespRight(0x1F, 5 - i), PAL_GREEN(col) | DespRight(0x1F, 5 - i), PAL_BLUE(col) | DespRight(0x1F, 5 - i));
}

void FadeStepColor(uint8_t i) {
	static palette_color_t palette[32];
	static palette_color_t palette_s[32];
	palette_color_t* col = (palette_color_t*)(&BkgPalette);
	palette_color_t* col_s = (palette_color_t*)(&SprPalette);

	for(uint8_t c = 0; c < 32; ++c, ++col, ++col_s) {
		palette[c] = UpdateColor(i, *col);
		palette_s[c] = UpdateColor(i, *col_s);
	};

	vsync();
	set_bkg_palette(0, 8, palette);
	set_sprite_palette(0, 8, palette_s);
	DISPLAY_ON;
	vsync();
}

void fade_out_modal(void) BANKED {
	if (_cpu == CGB_TYPE) {
		for(uint8_t i = 0; i != 6; i ++) FadeStepColor(i);
	} else {
		FadeDMG(FALSE);
	}
	DISPLAY_OFF;
}

void fade_in_modal(void) BANKED {
	if (_cpu == CGB_TYPE) {
		for(uint8_t i = 5; i != 0xFF; -- i) FadeStepColor(i);
	} else {
		DISPLAY_ON;
		FadeDMG(TRUE);
	}
}
