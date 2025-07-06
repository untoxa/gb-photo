#pragma bank 255

#include "fade.h"
#include "palette.h"

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
		vsync();
	}
}

void fade_out_modal(void) BANKED {
	FadeDMG(FALSE);
	DISPLAY_OFF;
}

void fade_in_modal(void) BANKED {
	DISPLAY_ON;
	FadeDMG(TRUE);
}

