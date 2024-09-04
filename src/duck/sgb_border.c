#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "compat.h"
#include "bankdata.h"

#include "sgb_border.h"

#define SGB_CHR_BLOCK0 0
#define SGB_CHR_BLOCK1 1

#define SGB_SCR_FREEZE 1
#define SGB_SCR_UNFREEZE 0

#define SGB_TRANSFER(buf,A,B) (buf[0]=(A),buf[1]=(B),sgb_transfer(buf))

#define MIN(A,B) ((A)<(B)?(A):(B))


static unsigned char map_buf[2];
static border_descriptor_t border_desc;

void set_sgb_border(const border_descriptor_t * desc, uint8_t bank) BANKED {
    banked_memcpy(&border_desc, desc, sizeof(border_desc), bank);

    SGB_TRANSFER(map_buf, (SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    uint8_t BGP_save = BGP_REG;

    BGP_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);
    SCX_REG = SCY_REG = 0U;

    uint8_t tmp_lcdc = LCDC_REG;

    LCDC_REG =  LCDCF_BG8000 | LCDCF_BG9800 | LCDCF_BGON | LCDCF_ON;

    // prepare tilemap for SGB_BORDER_CHR_TRN (should display all 256 tiles)

    uint8_t i = 0, j = 0, *dest = (uint8_t *)0x9800;
    do {
        while (STAT_REG & STATF_BUSY);      // wait for accessible VRAM
        *dest++ = i++;                      // write one byte
        if (++j == 20) dest += 12, j = 0;   // if row of 20 tiles written, jump to the next row
    } while (i);


    // transfer tile data
    banked_vmemcpy(_VRAM8000, (uint8_t *)border_desc.tiles, MIN(border_desc.tiles_size, 0x1000), border_desc.bank);
    SGB_TRANSFER(map_buf, (SGB_CHR_TRN << 3) | 1, SGB_CHR_BLOCK0);
    if (border_desc.tiles_size > 0x1000) {
        banked_vmemcpy(_VRAM8000, (uint8_t *)(border_desc.tiles + 0x1000), border_desc.tiles_size - 0x1000, border_desc.bank);
        SGB_TRANSFER(map_buf, (SGB_CHR_TRN << 3) | 1, SGB_CHR_BLOCK1);
    }

    // transfer map and palettes
    banked_vmemcpy(_VRAM8000, (uint8_t *)border_desc.map, border_desc.map_size, border_desc.bank);
    banked_vmemcpy(_VRAM8800, (uint8_t *)border_desc.palettes, border_desc.palettes_size, border_desc.bank);
    SGB_TRANSFER(map_buf, (SGB_PCT_TRN << 3) | 1, 0);

    // clear SCREEN
    vmemset(_VRAM8800, 0, 16);

    fill_bkg_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, 0x80);

    BGP_REG = BGP_save;

    sync_vblank();
    LCDC_REG = tmp_lcdc;

    SGB_TRANSFER(map_buf, (SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE);
}
