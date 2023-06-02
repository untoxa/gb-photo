#ifndef PALETTE_H
#define PALETTE_H

#include <gbdk/platform.h>
#include <stdint.h>

BANKREF_EXTERN(module_palette)

#define DMG_BLACK 0x03
#define DMG_DARK_GRAY 0x02
#define DMG_LITE_GRAY 0x01
#define DMG_WHITE 0x00

#ifndef DMG_PALETTE
#define DMG_PALETTE(C0, C1, C2, C3) ((uint8_t)((((C3) & 0x03) << 6) | (((C2) & 0x03) << 4) | (((C1) & 0x03) << 2) | ((C0) & 0x03)))
#endif

#define CGB_PALETTE(C0, C1, C2, C3) {C0, C1, C2, C3}
#define CGB_COLOR(R, G, B) ((uint16_t)(((R) & 0x1f) | (((G) & 0x1f) << 5) | (((B) & 0x1f) << 10)))

typedef struct palette_entry_t {
    uint16_t c0, c1, c2, c3;
} palette_entry_t;

#define BCPS_REG_ADDR 0x68
#define OCPS_REG_ADDR 0x6A

extern uint8_t DMG_palette[3];
extern palette_entry_t SprPalette[8];
extern palette_entry_t BkgPalette[8];

void palette_cgb_zero(UBYTE reg) OLDCALL BANKED;

void palette_reload(void) BANKED;
uint8_t INIT_module_palette(void) BANKED;

#endif
