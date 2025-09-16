#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "systemdetect.h"
#include "sgb_border.h"
#include "sgb_assets.h"

#include "gbcamera.h"
#include "state_camera.h"

#include "camera_sgb_border.h"
#include "camera_sgb_border_default.h"

BANKREF(module_sgb_assets)

static const uint8_t sgb_palette_gray[] = {
    SGB_PKT(SGB_PAL_01),
    SNES_LE(SNES_WHITE), SNES_LE(SNES_LIGHTGRAY), SNES_LE(SNES_DARKGRAY), SNES_LE(SNES_BLACK),
                         SNES_LE(SNES_LIGHTGRAY), SNES_LE(SNES_DARKGRAY), SNES_LE(SNES_BLACK)
};
static const uint8_t sgb_palette_red[] = {
    SGB_PKT(SGB_PAL_01),
    SNES_LE(RGB8(0xf3, 0xf8, 0xc3)), SNES_LE(RGB8(0xd1, 0xe4, 0x10)), SNES_LE(RGB8(0xe4, 0x3d, 0x20)), SNES_LE(SNES_BLACK),
                                     SNES_LE(SNES_LIGHTGRAY),         SNES_LE(SNES_DARKGRAY),          SNES_LE(SNES_BLACK)
};

static const uint8_t * const sgb_palettes[] = { sgb_palette_gray, sgb_palette_red };

static const border_descriptor_t sgb_borders[] = {
    {
        .tiles    = camera_sgb_border_default_tiles0,    .tiles_size    = size_camera_sgb_border_default_tiles0,
        .tiles_ex = camera_sgb_border_default_tiles1,    .tiles_ex_size = size_camera_sgb_border_default_tiles1,
        .map      = camera_sgb_border_default_map0,      .map_size      = size_camera_sgb_border_default_map0,
        .palettes = camera_sgb_border_default_palettes0, .palettes_size = size_camera_sgb_border_default_palettes0,
        .bank     = BANK(camera_sgb_border_default)
    }, {
        .tiles    = camera_sgb_border_tiles0,            .tiles_size    = size_camera_sgb_border_tiles0,
        .tiles_ex = camera_sgb_border_tiles1,            .tiles_ex_size = size_camera_sgb_border_tiles1,
        .map      = camera_sgb_border_map0,              .map_size      = size_camera_sgb_border_map0,
        .palettes = camera_sgb_border_palettes0,         .palettes_size = size_camera_sgb_border_palettes0,
        .bank     = BANK(camera_sgb_border)
    }
};

inline void sgb_assets_set_palette(uint8_t palette_index) {
    sgb_transfer((uint8_t *)sgb_palettes[palette_index]);
}

// load the SGB borders and palettes if SGB detected
uint8_t INIT_module_sgb_assets(void) BANKED {
    if (_is_SUPER) {
        CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
        if (OPTION(fancy_sgb_border)) {
            set_sgb_border(&sgb_borders[SGB_BORDER_FANCY], BANK(module_sgb_assets));
            sgb_assets_set_palette(SGB_PALETTE_RED);
        } else {
            set_sgb_border(&sgb_borders[SGB_BORDER_DEFAULT], BANK(module_sgb_assets));
            sgb_assets_set_palette(SGB_PALETTE_GRAY);
        }
    }
    return 0;
}