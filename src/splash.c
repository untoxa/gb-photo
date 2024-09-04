#pragma bank 1

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "systemdetect.h"
#include "joy.h"
#include "gbcamera.h"
#include "state_camera.h"
#include "splash.h"

#include "GBDK2020.h"   // logos must be compiled into the same bank

#define LENGTH(a) (sizeof((a))/sizeof((a)[0]))

#define WIDTH(a) (a ## _WIDTH / a ## _TILE_W)
#define HEIGHT(a) (a ## _HEIGHT / a ## _TILE_H)
#define CENTER_X(a) ((DEVICE_SCREEN_WIDTH - WIDTH(a)) / 2)
#define CENTER_Y(a) ((DEVICE_SCREEN_HEIGHT - HEIGHT(a)) / 2)
#define TILE_ORIGIN(a) (a ## _TILE_ORIGIN)

#if defined(NINTENDO)
#define TILE_SIZE(a) (a ## _TILE_H * 2)
#define TILE_VRAM _VRAM9000
#elif defined(SEGA)
#define TILE_SIZE(a) (a ## _TILE_H * 4)
#define TILE_VRAM (uint8_t *)(0x4000)
#endif

BANKREF(module_splash)

#if defined(NINTENDO)
static uint8_t mask_counter;
#endif
void apply_mask(uint8_t * sour, uint8_t *mask, uint8_t * dest) NAKED {
    sour; mask; dest;
__asm
#if defined(NINTENDO)
.macro WAIT_STAT ?lbl
lbl:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr nz, lbl
.endm
        ldhl sp, #2
        ld a, (hl+)
        ld h, (hl)
        ld l, c
        ld c, a
        ld a, h
        ld h, b
        ld b, a         ; source: de mask: hl dest: bc

        ld a, #8
1$:
        ld (_mask_counter), a

        .rept 2
            WAIT_STAT
            ld a, (de)
            and (hl)
            ld (bc), a
            inc c
            inc de
        .endm
        inc hl

        ld a, (_mask_counter)
        dec a
        jr nz, 1$

        pop hl
        pop de
        jp (hl)

#elif defined(SEGA)

.macro SWITCH_VBLANK_COPY DIS
        ld a, DIS
        ld (__shadow_OAM_OFF), a
.endm
.macro VDP_WRITE_CMD regH regL ?lbl
        ld a, regL
        di
        out (_VDP_CMD), a
        ld a, regH
        ei
        out (_VDP_CMD), a
.endm

        SWITCH_VBLANK_COPY #1

        pop bc
        ex (sp), hl

        VDP_WRITE_CMD h, l

        pop hl          ; function return void == callee cleanup stack
        push bc

        ld c, #8
1$:
        .rept 4
            ld a, (de)
            and (hl)
            out (_VDP_DATA), a
            inc hl
        .endm
        inc de

        dec c
        jp nz, 1$

        SWITCH_VBLANK_COPY #0

        ret
#endif
__endasm;
}

const mask_t mask_out = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x11,0x00,0x00,0x00,0x11,0x00},
    {0x44,0x00,0x11,0x00,0x44,0x00,0x11,0x00},
    {0x44,0xAA,0x11,0xAA,0x44,0xAA,0x11,0xAA},
    {0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA},
    {0x55,0xFF,0x55,0xFF,0x55,0xFF,0x55,0xFF},
    {0x77,0xFF,0xDD,0xFF,0x77,0xFF,0xDD,0xFF},
    {0x77,0xFF,0xFF,0xFF,0x77,0xFF,0xFF,0xFF}
};
const mask_t mask_in = {
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
    {0x77,0xFF,0xFF,0xFF,0x77,0xFF,0xFF,0xFF},
    {0x77,0xFF,0xDD,0xFF,0x77,0xFF,0xDD,0xFF},
    {0x55,0xFF,0x55,0xFF,0x55,0xFF,0x55,0xFF},
    {0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA},
    {0x44,0xAA,0x11,0xAA,0x44,0xAA,0x11,0xAA},
    {0x44,0x00,0x11,0x00,0x44,0x00,0x11,0x00},
    {0x00,0x00,0x11,0x00,0x00,0x00,0x11,0x00}
};

void logo_fade(const mask_t * mask) {
    static uint8_t i, j;
    static uint16_t offset;
    static uint8_t counters[WIDTH(GBDK2020)];

    for (i = 0; i != LENGTH(counters); i++)
        counters[i] = i + (WIDTH(GBDK2020) + 1);

    while (counters[LENGTH(counters) - 1]) {
        sync_vblank();
        sync_vblank();
        for (j = 0; j != WIDTH(GBDK2020); j++) {
            if (counters[j]) {
                if (--counters[j] < 8) {
                    offset = (j * TILE_SIZE(GBDK2020));
                    for (i = HEIGHT(GBDK2020); i != 0; i--, offset += (WIDTH(GBDK2020) * TILE_SIZE(GBDK2020)))
                        apply_mask(GBDK2020_tiles + offset, (*mask)[counters[j]], (TILE_VRAM + (TILE_ORIGIN(GBDK2020) * TILE_SIZE(GBDK2020))) + offset);
                }
            }
        }
    }
}

void logo_init(void) {
    DISPLAY_OFF;
    // set palettes if system supports color
    if (_is_COLOR) {
        set_bkg_palette(0, GBDK2020_PALETTE_COUNT, GBDK2020_palettes);
    }
    // we don't load tiles into VRAM, because they are loaded by the fading function
    // we set maps as if tiles are loaded
#if defined(NINTENDO)
    if (_is_COLOR) {
        VBK_REG = 1;
        fill_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, 0);
        set_tile_map(CENTER_X(GBDK2020), CENTER_Y(GBDK2020), WIDTH(GBDK2020), HEIGHT(GBDK2020), GBDK2020_map_attributes);
        VBK_REG = 0;
    }
#endif
    fill_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, 0);
    set_tile_map(CENTER_X(GBDK2020), CENTER_Y(GBDK2020), WIDTH(GBDK2020), HEIGHT(GBDK2020), GBDK2020_map);

    SHOW_BKG;
    DISPLAY_ON;
}

uint8_t INIT_module_splash(void) BANKED {
    // skip logo if fast boot
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    if (OPTION(boot_to_camera_mode)) return 0;
    // show logo
    logo_init();
    JOYPAD_RESET();
    logo_fade(&mask_in);
    for (uint8_t i = 0; i != 2 * 60; i++) {
        PROCESS_INPUT();
        if (KEY_PRESSED(J_ANY)) break; else sync_vblank();
    }
    logo_fade(&mask_out);
    return 0;
}