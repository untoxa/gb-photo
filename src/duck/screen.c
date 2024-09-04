#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "screen.h"
#include "gbcamera.h"
#include "systemdetect.h"

BANKREF(module_screen)
BANKREF(module_display_off)
BANKREF(module_display_on)

const uint8_t * const screen_tile_addresses[DEVICE_SCREEN_HEIGHT] = {
    TO_TILE_ADDRESS(TILE_BANK_0, 0x80),
    TO_TILE_ADDRESS(TILE_BANK_0, 0x94),
    TO_TILE_ADDRESS(TILE_BANK_0, 0xA8),
    TO_TILE_ADDRESS(TILE_BANK_0, 0xBC),
    TO_TILE_ADDRESS(TILE_BANK_0, 0xD0),
    TO_TILE_ADDRESS(TILE_BANK_0, 0xE4),
    TO_TILE_ADDRESS(TILE_BANK_0, 0x00),
    TO_TILE_ADDRESS(TILE_BANK_0, 0x14),
    TO_TILE_ADDRESS(TILE_BANK_0, 0x28),
    TO_TILE_ADDRESS(TILE_BANK_0, 0x3C),
    TO_TILE_ADDRESS(TILE_BANK_0, 0x50),
    TO_TILE_ADDRESS(TILE_BANK_0, 0x64),
    TO_TILE_ADDRESS(TILE_BANK_1, 0x00),
    TO_TILE_ADDRESS(TILE_BANK_1, 0x14),
    TO_TILE_ADDRESS(TILE_BANK_1, 0x28),
    TO_TILE_ADDRESS(TILE_BANK_1, 0x3C),
    TO_TILE_ADDRESS(TILE_BANK_1, 0x50),
    TO_TILE_ADDRESS(TILE_BANK_1, 0x64)
};

const uint8_t screen_tile_map[DEVICE_SCREEN_HEIGHT * DEVICE_SCREEN_WIDTH] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63,
    0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93,
    0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
    0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3,
    0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63,
    0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77
};

uint8_t * set_data_row(uint8_t * dest, const uint8_t * sour, uint8_t count) NAKED {
    dest; sour; count;
    __asm
.macro .WAIT_STAT_00 ?lbl
lbl:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr nz, lbl
.endm
        ldhl sp, #2
        ld a, (hl)

        ld h, d
        ld l, e

        ld d, a     ; d == count

1$:
        .rept 8
            ld a, (bc)
            inc bc
            ld e, a
            .WAIT_STAT_00
            ld (hl), e
            inc l
            ld a, (bc)
            ld (hl+), a
            inc bc
        .endm

        dec d
        jr nz, 1$

        pop hl
        inc sp
        jp (hl)
    __endasm;
}

static uint8_t hflip_loop;
uint8_t * set_data_row_flipped(uint8_t * dest, const uint8_t * sour, uint8_t count) NAKED {
    dest; sour; count;
    __asm
.macro .WAIT_STAT_03 ?lbl
lbl:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr nz, lbl
.endm
        ldhl sp, #2
        ld a, (hl)

        ld hl, #(0x10 - 2)
        add hl, de

1$:
        ld (#_hflip_loop), a

        .rept 8
            ld d, #>_flip_recode_table
            .rept 2
                ld a, (bc)
                inc bc
                ld e, a
                .WAIT_STAT_03
                ld a, (de)
                ld (hl+), a
            .endm
            ld de, #-4
            add hl, de
        .endm

        ld a, (#_hflip_loop)
        dec a
        jp nz, 1$

        pop hl
        inc sp
        jp (hl)
    __endasm;
}

inline const uint8_t * DMA_HBL_TRANSFER(uint8_t tiles, const uint8_t * sour, const uint8_t * dest) {
    rHDMA1 = (uint8_t)((uint16_t)sour >> 8), rHDMA2 = (uint8_t)sour;
    rHDMA3 = (uint8_t)((uint16_t)dest >> 8), rHDMA4 = (uint8_t)dest;
    rHDMA5 = (tiles - 1) | HDMA5F_MODE_HBL;
    return sour + ((uint16_t)tiles << 4);
}
void screen_transfer_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture) {
    const uint8_t *const *addr = (const uint8_t *const *)(screen_tile_addresses + y);
    uint16_t ofs = x << 4;
    do {
        picture = DMA_HBL_TRANSFER(w, picture, *addr++ + ofs);
        while (rHDMA5 != 0xff);
    } while (--h);
}
void screen_load_image_flipped(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture) {
    const uint8_t *const *addr = (const uint8_t *const *)(screen_tile_addresses + y + h - 1);
    uint16_t ofs = (x + w - 1) << 4;
    do {
        picture = set_data_row_flipped((uint8_t *)(*addr-- + ofs), picture, w);
    } while (--h);
}
void screen_load_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture) {
    const uint8_t *const *addr = (const uint8_t *const *)(screen_tile_addresses + y);
    uint16_t ofs = x << 4;
    do {
        picture = set_data_row((uint8_t *)(*addr++ + ofs), picture, w);
    } while (--h);
}

void screen_load_image_banked(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t * picture, uint8_t bank) {
    uint8_t save = _current_bank;
    CAMERA_SWITCH_ROM(bank);
    screen_load_image(x, y, w, h, picture);
    CAMERA_SWITCH_ROM(save);
}

void screen_load_tile_banked(uint8_t x, uint8_t y, uint8_t * tile, uint8_t bank) {
    uint8_t save = _current_bank;
    CAMERA_SWITCH_ROM(bank);
    set_data_row((uint8_t *)(*(uint8_t **)(screen_tile_addresses + y) + (x << 4)), tile, 1);
    CAMERA_SWITCH_ROM(save);
}

void screen_copy_thumbnail_row(uint8_t * dest, const uint8_t * sour) NAKED {
    dest; sour;
    __asm
.macro .WAIT_STAT_01 ?lbl
lbl:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr nz, lbl
.endm
        .rept 3
            .WAIT_STAT_01
            ld a, (bc)
            ld (de), a
            inc bc
            inc de
            .WAIT_STAT_01
            ld a, (bc)
            ld (de), a

            ld hl, #15
            add hl, bc
            ld b, h
            ld c, l

            ld hl, #15
            add hl, de
            ld d, h
            ld e, l
        .endm
        .WAIT_STAT_01
        ld a, (bc)
        ld (de), a
        inc bc
        inc de
        .WAIT_STAT_01
        ld a, (bc)
        ld (de), a
        ret
    __endasm;
}

void screen_clear_thumbnail_row(uint8_t * dest, uint8_t fill) NAKED {
    dest; fill;
    __asm
.macro .WAIT_STAT_02 ?lbl
lbl:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr nz, lbl
.endm
        ld c, a
        .rept 3
            .WAIT_STAT_02
            ld a, c
            ld (de), a
            inc de
            .WAIT_STAT_02
            ld a, c
            ld (de), a

            ld hl, #15
            add hl, de
            ld d, h
            ld e, l
        .endm
        .WAIT_STAT_02
        ld a, c
        ld (de), a
        inc de
        .WAIT_STAT_02
        ld a, c
        ld (de), a
        ret
    __endasm;
}


void screen_load_thumbnail(uint8_t x, uint8_t y, uint8_t * picture, uint8_t fill) {
    uint8_t * dest, *sour;
    for (uint8_t i = 0; i != 32; i++) {
        dest = (uint8_t *)(screen_tile_addresses[y + (i  / 8)] + (x * 16) + ((i % 8) << 1));
        if (i < 2 || i > 29) {
            screen_clear_thumbnail_row(dest, fill);
        } else {
            sour = picture + ((i - 2) / 8) * (CAMERA_THUMB_TILE_WIDTH * 16) + (((i - 2) % 8) << 1);
            screen_copy_thumbnail_row(dest, sour);
        }
    }
}

void screen_load_thumbnail_banked(uint8_t x, uint8_t y, uint8_t * picture, uint8_t fill, uint8_t bank) {
    uint8_t save = _current_bank;
    CAMERA_SWITCH_ROM(bank);
    screen_load_thumbnail(x, y, picture, fill);
    CAMERA_SWITCH_ROM(save);
}

void LCD_ISR(void) NONBANKED {
    while (STAT_REG & STATF_BUSY);
    if (LYC_REG == 95) {
        LCDC_REG |= LCDCF_BG8000;
        LYC_REG = 143;
    } else {
        LCDC_REG &= ~LCDCF_BG8000;
        LYC_REG = 95;
    }
}

uint8_t INIT_module_screen(void) BANKED {
    // (re)install interrupt handlers
    CRITICAL {
        LYC_REG = 95, STAT_REG |= STATF_LYC;
        remove_LCD(LCD_ISR);
        add_LCD(LCD_ISR);
    }
    set_interrupts(IE_REG | LCD_IFLAG);
    // prepare the screen map
    if (_is_COLOR) {
        VBK_REG = 1;
        fill_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, 0);
        VBK_REG = 0;
    }
    screen_clear_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);
    return 0;
}

// switch display off
uint8_t INIT_module_display_off(void) BANKED {
    DISPLAY_OFF;
    return 0;
}

// set LCDC and switch on display
uint8_t INIT_module_display_on(void) BANKED {
    LCDC_REG |= (LCDCF_ON | LCDCF_BGON | LCDCF_OBJON | LCDCF_OBJ8);
    return 0;
}