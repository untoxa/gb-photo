#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "gbprinter.h"
#include "gbcamera.h"
#include "bankdata.h"
#include "states.h"
#include "globals.h"

#include "state_camera.h"

#define START_TRANSFER 0x81
#if (CGB_FAST_TRANSFER==1)
    // 0b10000011 - start, CGB double speed, internal clock
    #define START_TRANSFER_FAST 0x83
#else
    // 0b10000001 - start, internal clock
    #define START_TRANSFER_FAST 0x81
#endif

static const uint8_t PRN_PKT_INIT[]    = { PRN_MAGIC_1,PRN_MAGIC_2,PRN_CMD_INIT,0x00,0x00,0x00,0x01,0x00,0x00,0x00 };
static const uint8_t PRN_PKT_STATUS[]  = { PRN_MAGIC_1,PRN_MAGIC_2,PRN_CMD_STATUS,0x00,0x00,0x00,0x0F,0x00,0x00,0x00 };
static const uint8_t PRN_PKT_EOF[]     = { PRN_MAGIC_1,PRN_MAGIC_2,PRN_CMD_DATA,0x00,0x00,0x00,0x04,0x00,0x00,0x00 };

start_print_pkt_t PRN_PKT_START = {
    .magic = PRN_MAGIC, .command = PRN_CMD_PRINT, .length = 4,
    .print = TRUE, .margins = 0, .palette = PRN_PALETTE_NORMAL, .exposure = PRN_EXPOSURE_DARK,
    .crc = 0, .trail = 0
};

uint16_t printer_status;
uint8_t printer_tile_num;

uint8_t printer_completion = 0;
far_ptr_t printer_progress_handler = {0, NULL};

uint8_t printer_send_receive(uint8_t b) {
    SB_REG = b;
    SC_REG = (OPTION(print_fast)) ? START_TRANSFER_FAST : START_TRANSFER;
    while (SC_REG & 0x80);
    return SB_REG;
}

uint8_t printer_send_byte(uint8_t b) {
    return (uint8_t)(printer_status = ((printer_status << 8) | printer_send_receive(b)));
}

uint8_t printer_send_command(const uint8_t *command, uint8_t length) {
    uint8_t index = 0;
    while (index++ < length) printer_send_byte(*command++);
    return ((uint8_t)(printer_status >> 8) == 0x81) ? (uint8_t) printer_status : PRN_STATUS_MASK_ERRORS;
}
#define PRINTER_SEND_COMMAND(CMD) printer_send_command((const uint8_t *)&(CMD), sizeof(CMD))

uint8_t printer_print_tile(const uint8_t *tiledata) {
    static const uint8_t PRINT_TILE[] = { 0x88,0x33,0x04,0x00,0x80,0x02 };
    static uint16_t printer_CRC;
    if (printer_tile_num == 0) {
        const uint8_t * data = PRINT_TILE;
        for (uint8_t i = sizeof(PRINT_TILE); i != 0; i--) printer_send_receive(*data++);
        printer_CRC = 0x04 + 0x80 + 0x02;
    }
    for(uint8_t i = 0x10; i != 0; i--, tiledata++) {
        printer_CRC += *tiledata;
        printer_send_receive(*tiledata);
    }
    if (++printer_tile_num == 40) {
        printer_send_receive((uint8_t)printer_CRC);
        printer_send_receive((uint8_t)(printer_CRC >> 8));
        printer_send_receive(0x00);
        printer_send_receive(0x00);
        printer_CRC = printer_tile_num = 0;
        return TRUE;
    }
    return FALSE;
}

inline void printer_init() {
    printer_tile_num = 0;
    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
}

uint8_t printer_wait(uint16_t timeout, uint8_t mask, uint8_t value) {
    uint8_t error;
    while (((error = PRINTER_SEND_COMMAND(PRN_PKT_STATUS)) & mask) != value) {
        if (timeout-- == 0) return PRN_STATUS_MASK_ERRORS;
        if (error & PRN_STATUS_MASK_ERRORS) break;
        wait_vbl_done();
    }
    return error;
}

uint8_t gbprinter_detect(uint8_t delay) BANKED {
    printer_init();
    return printer_wait(delay, PRN_STATUS_MASK_ANY, PRN_STATUS_OK);
}

uint8_t gbprinter_print_image(const uint8_t * image, uint8_t image_bank, const frame_desc_t * frame, uint8_t frame_bank) BANKED {
    static frame_desc_t current_frame;
    static const uint8_t * img;

    printer_completion = 0;
    call_far(&printer_progress_handler);

    banked_memcpy(&current_frame, frame, sizeof(current_frame), frame_bank);

    uint8_t tile_data[16], error, packets;

    if ((packets = current_frame.height >> 1) == 0) return PRN_STATUS_OK;

    SWITCH_RAM(image_bank);
    img = image;

    const uint8_t * map = current_frame.map, row_count = packets << 1;
    printer_tile_num = 0;

    gbprinter_set_print_params(0, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
    for (uint8_t y = 0; y != row_count; y++) {
        for (uint8_t x = 0; x != 20; x++, map++) {
            // copy frame tile if applicable
            if (current_frame.map) {
                uint8_t tileno = read_banked_ubyte(map, current_frame.map_bank);
                banked_memcpy(tile_data, current_frame.tiles + ((uint16_t)tileno << 4), sizeof(tile_data), current_frame.tiles_bank);
            } else memset(tile_data, 0, sizeof(tile_data));
            // overlay the picture tile if in range
            if ((y >= current_frame.image_y) && (y < (current_frame.image_y + CAMERA_IMAGE_TILE_HEIGHT)) &&
                (x >= current_frame.image_x) && (x < (current_frame.image_x + CAMERA_IMAGE_TILE_WIDTH))) {
                memcpy(tile_data, img + ((((y - current_frame.image_y) << 4) + (x - current_frame.image_x)) << 4), sizeof(tile_data));
            }
            // print the resulting tile
            if (printer_print_tile(tile_data)) {
                PRINTER_SEND_COMMAND(PRN_PKT_EOF);
                // setup printing if required
                if (y == (row_count - 1)) gbprinter_set_print_params(3, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
                PRINTER_SEND_COMMAND(PRN_PKT_START);
                // query printer status
                if ((error = printer_wait(SECONDS(1), PRN_STATUS_BUSY, PRN_STATUS_BUSY)) & PRN_STATUS_MASK_ERRORS) return error;
                if ((error = printer_wait(SECONDS(10), PRN_STATUS_BUSY, 0)) & PRN_STATUS_MASK_ERRORS) return error;

                uint8_t current_progress = ((uint16_t)y << 3) / row_count;
                if (printer_completion != current_progress) {
                    printer_completion = current_progress, call_far(&printer_progress_handler);
                }
            }
        }
    }
    return PRINTER_SEND_COMMAND(PRN_PKT_STATUS);
}

