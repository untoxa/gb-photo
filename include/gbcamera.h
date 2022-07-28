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

/** Magic (in ASCII) is a mandatory keyword in the sram
    It precedes any checksum
*/
#define CAMERA_MAGIC "Magic"

/** Images are stored in 30 sram slots and the state of each slot is stored in a vector state of 30 bytes, in the same order
    0xFF means that the slot is free or erased, any number between 0x00 and 0x1D indicates the # of image in the album corresponding to the # of the memory slot
    Deleting an image replace its value by 0xFF but does not erase the memory slot.
*/
#define CAMERA_IMAGE_DELETED 0xFF

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

#define CAMERA_BANK_LAST_SEEN 0

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

/** The Mitsubishi M64282FP artificial retina of the Game Boy Camera is driven by filling 8 registers of 1 byte each
    The MAC-GBD only accept 5 registers for image settings (+1 for com, not counted), so 3 are probably forced by the MAC-GBD and not editable.
    (https://github.com/AntonioND/gbcam-rev-engineer)
    (https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor)
    0xA001 (CAM01) = M64282FP register 1: exclusively set edge enhancement mode (1 bit), vertical/horizontal edge enhancement mode (1 bit) and gain (5 bits)
    0xA002 (CAM02) = M64282FP register 2: exposure time, raw, from 4 ms to 1 second (8 bits)
    0xA003 (CAM03) = M64282FP register 3: exposure time, fine, from 0 to 4 ms (8 bits) - value below 0.25 ms create vertical artifacts
    M64282FP register 4: 1D filtering kernel P (8 bits) - not editable - set to 0x01 by Game Boy Camera MAC-GBD
    M64282FP register 5: 1D filtering kernel M (8 bits) - not editable - set to 0x00 by Game Boy Camera MAC-GBD
    M64282FP register 6: 1D filtering kernel X (8 bits) - not editable - set to 0x01 by Game Boy Camera MAC-GBD
    0xA004 (CAM04) = M64282FP register 7: Edge engancement ratio (4 bits), inverted output (1 bit), output bias voltage (3 bits)
    0xA005 (CAM05) = M64282FP register 0: zero point calibration (2 bits), output reference voltage (6 bits)

    0x0000 (CAM0) is an exchange register
 */

#define CAMERA_BANK_REGISTERS 16

// Camera hardware register: Capture
#define CAM00F_POSITIVE     0b00000010
#define CAM00F_NEGATIVE     0b00000000
#define CAM00F_CAPTURING    0b00000001

static volatile uint8_t AT(0xA000) CAM00_REG;
static volatile uint8_t AT(0xA000) CAM_REG_CAPTURE;

// Camera hardware register: EdgeExclusive, EdgeOperation, Gain

#define CAM01_EDGEOP_NONE   0b00000000
#define CAM01_EDGEOP_HORIZ  0b00100000
#define CAM01_EDGEOP_VERT   0b01000000
#define CAM01_EDGEOP_2D     0b01100000

#define CAM01F_EDGEEXCL_V_ON    0b10000000
#define CAM01F_EDGEEXCL_V_OFF   0b00000000

// The Game Boy Camera uses 0x00, 0x04, 0x08 and 0x0C
// They are 14.0dB, 20.0dB, 26.0dB and 32dB, which translate to a gain of 5.01, 10.00, 19.95 and 39.81.
#define CAM01_GAIN_140   0b00000000 // 14.0 (gbcam gain:  5.01)
#define CAM01_GAIN_155   0b00000001 // 15.5
#define CAM01_GAIN_170   0b00000010 // 17.0
#define CAM01_GAIN_185   0b00000010 // 18.5
#define CAM01_GAIN_200   0b00000100 // 20.0 (gbcam gain: 10.00)
#define CAM01_GAIN_200_D 0b00010000 // 20.0 (d)
#define CAM01_GAIN_215   0b00000101 // 21.5
#define CAM01_GAIN_215_D 0b00010001 // 21.5 (d)
#define CAM01_GAIN_230   0b00000110 // 23.0
#define CAM01_GAIN_230_D 0b00010010 // 23.0 (d)
#define CAM01_GAIN_245   0b00000111 // 24.5
#define CAM01_GAIN_245_D 0b00010010 // 24.5 (d)
#define CAM01_GAIN_260   0b00001000 // 26.0 (gbcam gain: 19.95)
#define CAM01_GAIN_260_D 0b00010100 // 26.0 (d)
#define CAM01_GAIN_275   0b00010101 // 27.5
#define CAM01_GAIN_290   0b00001001 // 29.0
#define CAM01_GAIN_290_D 0b00010110 // 29.0 (d)
#define CAM01_GAIN_305   0b00010111 // 30.5
#define CAM01_GAIN_320   0b00001010 // 32.0 (gbcam gain: 39.81)
#define CAM01_GAIN_320_D 0b00011000 // 32.0 (d)
#define CAM01_GAIN_350   0b00001011 // 35.0
#define CAM01_GAIN_350_D 0b00011001 // 35.0 (d)
#define CAM01_GAIN_380   0b00001100 // 38.0
#define CAM01_GAIN_380_D 0b00011010 // 38.0 (d)
#define CAM01_GAIN_410   0b00001101 // 41.0
#define CAM01_GAIN_410_D 0b00011011 // 41.0 (d)
#define CAM01_GAIN_440   0b00011100 // 44.0
#define CAM01_GAIN_455   0b00001110 // 45.5
#define CAM01_GAIN_470   0b00011101 // 47.0
#define CAM01_GAIN_515   0b00001111 // 51.5
#define CAM01_GAIN_515_D 0b00011110 // 51.5 (d)
#define CAM01_GAIN_575   0b00011111 // 57.5

static volatile uint8_t AT(0xA001) CAM01_REG;
static volatile uint8_t AT(0xA001) CAM_REG_EDEXOPGAIN;

// Camera hardware register: Exposure Time

#define US_TO_EXPOSURE_VALUE(A) ((uint16_t)((((A) >> 4) >> 8) | ((((A) >> 4) & 0xFF) << 8)))
#define EXPOSURE_VALUE_TO_US(A) ((uint32_t)(((A) >> 8) | (((A) & 0xFF) << 8)) << 4)

static volatile uint16_t AT(0xA002) CAM02_REG;
static volatile uint16_t AT(0xA002) CAM_REG_EXPTIME;

// Camera hardware register: Edge Ratio, Invert Output, Voltage Ref
#define CAM04F_INV  0b00001000
#define CAM04F_POS  0b00000000

#define CAM04_EDGE_RATIO_050    0b00000000
#define CAM04_EDGE_RATIO_075    0b00010000
#define CAM04_EDGE_RATIO_100    0b00100000
#define CAM04_EDGE_RATIO_125    0b00110000
#define CAM04_EDGE_RATIO_200    0b01000000
#define CAM04_EDGE_RATIO_300    0b01010000
#define CAM04_EDGE_RATIO_400    0b01100000
#define CAM04_EDGE_RATIO_500    0b01110000

#define CAM04_VOLTAGE_REF_00    0b00000000
#define CAM04_VOLTAGE_REF_05    0b00000001
#define CAM04_VOLTAGE_REF_10    0b00000010
#define CAM04_VOLTAGE_REF_15    0b00000011
#define CAM04_VOLTAGE_REF_20    0b00000100
#define CAM04_VOLTAGE_REF_25    0b00000101
#define CAM04_VOLTAGE_REF_30    0b00000110
#define CAM04_VOLTAGE_REF_35    0b00000111


static volatile uint8_t AT(0xA004) CAM04_REG;
static volatile uint8_t AT(0xA004) CAM_REG_EDRAINVVREF;

// Camera hardware register: Zero Points, Voltage Out
#define CAM05_ZERO_DIS  0b00000000
#define CAM05_ZERO_POS  0b10000000
#define CAM05_ZERO_NEG  0b01000000

#define MIN_VOLTAGE_OUT -992
#define MAX_VOLTAGE_OUT 992
#define VOLTAGE_OUT_STEP 32
#define TO_VOLTAGE_OUT(V) (((V) < 0)?((~((int8_t)((V) >> 5)) + 1) & 0x1f) : (((int8_t)((V) >> 5) & 0x1f) | 0x20))

static volatile uint8_t AT(0xA005) CAM05_REG;
static volatile uint8_t AT(0xA005) CAM_REG_ZEROVOUT;

// Camera hardware register: Dither Pattern (48 bytes)
static uint8_t AT(0xA006) CAM_DITHERPATTERN[48];

#endif
