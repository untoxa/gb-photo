#pragma bank 255

#include <string.h>

#include "gbprinter.h"
#include "gbcamera.h"
#include "bankdata.h"

const uint8_t PRINTER_INIT[]    = { sizeof(PRINTER_INIT),  0x88,0x33,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00 };
const uint8_t PRINTER_STATUS[]  = { sizeof(PRINTER_STATUS),0x88,0x33,0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00 };
const uint8_t PRINTER_EOF[]     = { sizeof(PRINTER_EOF),   0x88,0x33,0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00 };
const uint8_t PRINTER_START[]   = { sizeof(PRINTER_START), 0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x03,0xE4,0x7F,0x6D,0x01,0x00,0x00 };

uint16_t printer_status;
uint8_t printer_tile_num;

uint8_t printer_send_receive(uint8_t b) {
    SB_REG = b;             // data to send
    SC_REG = 0x81;          // 1000 0001 - start, internal clock
    while (SC_REG & 0x80);  // wait until b1 reset
    return SB_REG;          // return response stored in SB_REG
}

uint8_t printer_send_byte(uint8_t b) {
    return (uint8_t)(printer_status = ((printer_status << 8) | printer_send_receive(b)));
}

uint8_t printer_send_command(const uint8_t *command) {
    uint8_t index = 0, length = *command++ - 1;
    while (index++ < length) printer_send_byte(*command++);
    return ((uint8_t)(printer_status >> 8) == 0x81) ? (uint8_t) printer_status : STATUS_MASK_ERRORS;
}

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
    printer_send_command(PRINTER_INIT);
}

uint8_t printer_wait(uint16_t timeout, uint8_t mask, uint8_t value) {
    uint8_t error;
    while (((error = printer_send_command(PRINTER_STATUS)) & mask) != value) {
        if (timeout-- == 0) return STATUS_MASK_ERRORS;
        if (error & STATUS_MASK_ERRORS) break;
        wait_vbl_done();
    }
    return error;
}

uint8_t gbprinter_detect(uint8_t delay) BANKED {
    printer_init();
    return printer_wait(delay, STATUS_MASK_ANY, STATUS_OK);
}

uint8_t gbprinter_print_image(const uint8_t n, const frame_desc_t * frame, uint8_t frame_bank) BANKED {
    static frame_desc_t current_frame;

    banked_memcpy(&current_frame, frame, sizeof(current_frame), frame_bank);

    uint8_t tile_data[16], error, packets;

    if ((packets = current_frame.height >> 1) == 0) return STATUS_OK;

    if (n > (CAMERA_MAX_IMAGE_SLOTS - 1)) return STATUS_MASK_ERRORS;
    SWITCH_RAM((n >> 1) + 1);

    uint8_t * image = ((n & 1) ? image_second : image_first);

    const uint8_t * map = current_frame.map;
    printer_tile_num = 0;
    for (uint8_t y = 0; y != packets << 1; y++) {
        for (uint8_t x = 0; x != 20; x++, map++) {
            // copy frame tile if applicable
            if (current_frame.map) {
                uint8_t tileno = read_banked_ubyte(map, current_frame.map_bank);
                banked_memcpy(tile_data, current_frame.tiles + ((uint16_t)tileno << 4), sizeof(tile_data), current_frame.tiles_bank);
            } else memset(tile_data, 0, sizeof(tile_data));
            // overlay the picture tile if in range
            if ((y >= current_frame.image_y) && (y < (current_frame.image_y + CAMERA_IMAGE_TILE_HEIGHT)) && 
                (x >= current_frame.image_x) && (x < (current_frame.image_x + CAMERA_IMAGE_TILE_WIDTH))) {
                memcpy(tile_data, image + ((((y - current_frame.image_y) << 4) + (x - current_frame.image_x)) << 4), sizeof(tile_data));
            }
            // print the resulting tile
            if (printer_print_tile(tile_data)) {
                printer_send_command(PRINTER_EOF);
                printer_send_command(PRINTER_START);
                printer_send_command(PRINTER_STATUS);
                if ((error = printer_wait(SECONDS(1), STATUS_BUSY, STATUS_BUSY)) & STATUS_MASK_ERRORS) return error;
                if ((error = printer_wait(SECONDS(10), STATUS_BUSY, 0)) & STATUS_MASK_ERRORS) return error;
            }
        }
    }
    return printer_send_command(PRINTER_STATUS);
}

