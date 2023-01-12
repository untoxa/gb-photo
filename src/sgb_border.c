#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "sgb_border.h"

#define SGB_CHR_BLOCK0 0
#define SGB_CHR_BLOCK1 1

#define SGB_SCR_FREEZE 1
#define SGB_SCR_UNFREEZE 0

#define SGB_TRANSFER(A,B) map_buf[0]=(A),map_buf[1]=(B),sgb_transfer(map_buf)

#define MIN(A,B) ((A)<(B)?(A):(B))

void set_sgb_border(const uint8_t * tiledata, size_t tiledata_size,
                    const uint8_t * tilemap,  size_t tilemap_size,
                    const uint8_t * palette,  size_t palette_size,
                    uint8_t bank) NONBANKED {

    uint8_t save = _current_bank;
    SWITCH_ROM(bank);

    unsigned char map_buf[20];
    memset(map_buf, 0, sizeof(map_buf));

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    BGP_REG = OBP0_REG = OBP1_REG = 0xE4U;
    SCX_REG = SCY_REG = 0U;

    uint8_t tmp_lcdc = LCDC_REG;

    LCDC_REG =  LCDCF_BG8000 | LCDCF_BG9800 | LCDCF_BGON | LCDCF_ON;

    // prepare tilemap for SGB_BORDER_CHR_TRN (should display all 256 tiles)
    uint8_t i = 0;
    for (uint8_t y = 0; y != 14; y++) {
        for (uint8_t x = 20, * dout = map_buf; (x); x--) {
            *dout++ = i++;
        }
        set_bkg_tiles(0, y, 20, 1, map_buf);
    }
    memset(map_buf, 0, sizeof(map_buf));

    // transfer tile data
    vmemcpy(_VRAM8000, (uint8_t *)tiledata, MIN(tiledata_size, 0x1000));
    SGB_TRANSFER((SGB_CHR_TRN << 3) | 1, SGB_CHR_BLOCK0);
    if (tiledata_size > 0x1000) {
        vmemcpy(_VRAM8000, (uint8_t *)(tiledata + 0x1000), tiledata_size - 0x1000);
        SGB_TRANSFER((SGB_CHR_TRN << 3) | 1, SGB_CHR_BLOCK1);
    }

    // transfer map and palettes
    vmemcpy(_VRAM8000, (uint8_t *)tilemap, tilemap_size);
    vmemset(_VRAM8800, 0, (4 * 16 * 2));
    vmemcpy(_VRAM8800, (uint8_t *)palette, palette_size);
    SGB_TRANSFER((SGB_PCT_TRN << 3) | 1, 0);

    // clear SCREEN
    vmemset(_VRAM8800, 0, 16);
    fill_bkg_rect(0, 0, 20, 18, 0x80);

    vsync();
    LCDC_REG = tmp_lcdc;

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE);

    SWITCH_ROM(save);
}
