#ifndef __GBCAMERA_H_INCLUDE__
#define __GBCAMERA_H_INCLUDE__

#include <types.h>
#include <stdint.h>

#define CAMERA_MAX_IMAGE_SLOTS 30

#define CAMERA_IMAGE_TILE_WIDTH 16
#define CAMERA_IMAGE_TILE_HEIGHT 14

#define CAMERA_THUMB_TILE_WIDTH 4
#define CAMERA_THUMB_TILE_HEIGHT 4

#define CAMERA_IMAGE_SIZE (CAMERA_IMAGE_TILE_WIDTH * CAMERA_IMAGE_TILE_HEIGHT * 16)
#define CAMERA_THUMB_SIZE (CAMERA_THUMB_TILE_WIDTH * CAMERA_THUMB_TILE_HEIGHT * 16)

#define CAMERA_MAGIC "Magic"

typedef struct cam_game_data_t {
    uint8_t imageslots[CAMERA_MAX_IMAGE_SLOTS];
    uint8_t magic[5];
    uint8_t CRC_add;
    uint8_t CRC_xor;
} cam_game_data_t;

/*
 * Area 0x0000 to 0x1FFF in RAM
 * Using RAM Bank 0
 * Containing Last seen image, gameface data etc.
 */

//static uint8_t AT(0xA000) last_seen_upper_unused[256];
static uint8_t AT(0xA100) last_seen[CAMERA_IMAGE_SIZE];
static uint8_t AT(0xA100) last_seen_upper[CAMERA_IMAGE_SIZE >> 1];
static uint8_t AT(0xA800) last_seen_lower[CAMERA_IMAGE_SIZE >> 1];
//static uint8_t AT(0xAF00) last_seen_padding[256];
//static uint8_t AT(0xB000) game_data_meta_pad_0[434];

static cam_game_data_t AT(0xB1B2) cam_game_data;
static cam_game_data_t AT(0xB1D7) cam_game_data_echo;

//static uint8_t AT(0xB1FC) game_data_meta[3588];

/*
 * The 30 actual images in the camera's RAM
 * Area from 0x2000 to end in RAM
 * In steps of 0x2000 bytes using Banks 1 to 15
 * Each area containing two images (first/secons) including metadata, thumbnail etc
 * https://funtography.online/wiki/Cartridge_RAM
 */
static uint8_t AT(0xA000) image_first[CAMERA_IMAGE_SIZE];
static uint8_t AT(0xA000) image_first_upper[CAMERA_IMAGE_SIZE >> 1];
static uint8_t AT(0xA700) image_first_lower[CAMERA_IMAGE_SIZE >> 1];
static uint8_t AT(0xAE00) image_first_thumbnail[CAMERA_THUMB_SIZE];
static uint8_t AT(0xAF00) image_first_meta[92];
static uint8_t AT(0xAF5C) image_first_meta_echo[92];
// static uint8_t AT(0xAFB8) image_first_padding[51];
 static uint8_t AT(0xAFEB) image_first_unused[21]; // THIS AREA IS UNUSED BY THE ORIGINAL ROM - PXLR Stores data here in RAM bank 1!

static uint8_t AT(0xB000) image_second[CAMERA_IMAGE_SIZE];
static uint8_t AT(0xB000) image_second_upper[CAMERA_IMAGE_SIZE >> 1];
static uint8_t AT(0xB700) image_second_lower[CAMERA_IMAGE_SIZE >> 1];
static uint8_t AT(0xBE00) image_second_thumbnail[CAMERA_THUMB_SIZE];
static uint8_t AT(0xBF00) image_second_meta[92];
static uint8_t AT(0xBF5C) image_second_meta_echo[92];
// static uint8_t AT(0xBFB8) image_second_padding[51];
// static uint8_t AT(0xBFEB) image_second_unused[21]; // THIS AREA IS UNUSED BY THE ORIGINAL ROM - PXLR may store data here in the future

/*
 * 0x0000 to 0x0035
 * Must be written to RAM bank 16 to control the camera's sensor
 */

// Camera hardware register: Capture
static volatile uint8_t AT(0xA000) CAM_REG_CAPTURE;
// Camera hardware register: EdgeExclusive, EdgeOperation, Gain
static uint8_t AT(0xA001) CAM_REG_EDEXOPGAIN;
// Camera hardware register: Exposure Time
static uint16_t AT(0xA002) CAM_REG_EXPTIME;
// Camera hardware register: Edge Ratio, Invert Output, Voltage Ref
static uint8_t AT(0xA004) CAM_REG_EDRAINVVREF;
// Camera hardware register: Zero Points, Voltage Out
static uint8_t AT(0xA005) CAM_REG_ZEROVOUT;
// Camera hardware register: Dither Pattern (48 bytes)
static uint8_t AT(0xA006) CAM_REG_DITHERPATTERN[48];

#endif
