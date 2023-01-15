#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "compat.h"
#include "sgb_border.h"

#define SGB_CHR_BLOCK0 0
#define SGB_CHR_BLOCK1 1

#define SGB_SCR_FREEZE 1
#define SGB_SCR_UNFREEZE 0

#define SGB_TRANSFER(buf,A,B) (buf[0]=(A),buf[1]=(B),sgb_transfer(buf))

#define MIN(A,B) ((A)<(B)?(A):(B))

void set_sgb_border(const uint8_t * tiledata, size_t tiledata_size,
                    const uint8_t * tilemap,  size_t tilemap_size,
                    const uint8_t * palette,  size_t palette_size,
                    uint8_t bank) NONBANKED {

    uint8_t save = _current_bank;
    SWITCH_ROM(bank);

    unsigned char map_buf[2];

    SGB_TRANSFER(map_buf, (SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    BGP_REG = OBP0_REG = OBP1_REG = 0xE4U;
    SCX_REG = SCY_REG = 0U;

    uint8_t tmp_lcdc = LCDC_REG;

    LCDC_REG =  LCDCF_BG8000 | LCDCF_BG9800 | LCDCF_BGON | LCDCF_ON;

    // prepare tilemap for SGB_BORDER_CHR_TRN (should display all 256 tiles)

    uint8_t i = 0, j = 0, *dest = 0x9800;
    do {
        while (STAT_REG & STATF_BUSY);      // wait for accessible VRAM
        *dest++ = i++;                      // write one byte
        if (++j == 20) dest += 12, j = 0;   // if row of 20 tiles written, jump to the next row
    } while (i);


    // transfer tile data
    vmemcpy(_VRAM8000, (uint8_t *)tiledata, MIN(tiledata_size, 0x1000));
    SGB_TRANSFER(map_buf, (SGB_CHR_TRN << 3) | 1, SGB_CHR_BLOCK0);
    if (tiledata_size > 0x1000) {
        vmemcpy(_VRAM8000, (uint8_t *)(tiledata + 0x1000), tiledata_size - 0x1000);
        SGB_TRANSFER(map_buf, (SGB_CHR_TRN << 3) | 1, SGB_CHR_BLOCK1);
    }

    // transfer map and palettes
    vmemcpy(_VRAM8000, (uint8_t *)tilemap, tilemap_size);
    vmemcpy(_VRAM8800, (uint8_t *)palette, palette_size);
    SGB_TRANSFER(map_buf, (SGB_PCT_TRN << 3) | 1, 0);

    // clear SCREEN
    vmemset(_VRAM8800, 0, 16);

    fill_bkg_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, 0x80);

    vsync();
    LCDC_REG = tmp_lcdc;

    SGB_TRANSFER(map_buf, (SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE);

    SWITCH_ROM(save);
}
