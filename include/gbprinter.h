#ifndef __GBPRINTER_H_INCLUDE__
#define __GBPRINTER_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#include "bankdata.h"
#include "print_frames.h"

#define PRN_LOW(A) ((A) & 0xFF)
#define PRN_HIGH(A) ((A) >> 8)

#define PRN_MAGIC               0x3388
#define PRN_MAGIC_1             PRN_LOW(PRN_MAGIC)
#define PRN_MAGIC_2             PRN_HIGH(PRN_MAGIC)

#define PRN_CMD_INIT            0x01
#define PRN_CMD_PRINT           0x02
#define PRN_CMD_DATA            0x04
#define PRN_CMD_STATUS          0x0F

#define PRN_PALETTE_NORMAL      0b11100100u
#define PRN_PALETTE_INV         0b00011011u

#define PRN_EXPOSURE_LIGHT      0x00
#define PRN_EXPOSURE_DEFAULT    0x40
#define PRN_EXPOSURE_DARK       0x7F

#define PRN_STATUS_LOWBAT       0x80
#define PRN_STATUS_ER2          0x40
#define PRN_STATUS_ER1          0x20
#define PRN_STATUS_ER0          0x10
#define PRN_STATUS_UNTRAN       0x08
#define PRN_STATUS_FULL         0x04
#define PRN_STATUS_BUSY         0x02
#define PRN_STATUS_SUM          0x01
#define PRN_STATUS_OK           0x00

#define PRN_STATUS_MASK_ERRORS  0xF0
#define PRN_STATUS_MASK_ANY     0xFF

#define SECONDS(A)              ((A)*60)

#define PRN_MAX_PROGRESS        8

typedef struct start_print_pkt_t {
    uint16_t magic;
    uint16_t command;
    uint16_t length;
    uint8_t print;
    uint8_t margins;
    uint8_t palette;
    uint8_t exposure;
    uint16_t crc;
    uint16_t trail;
} start_print_pkt_t;

extern uint8_t printer_completion;
extern far_ptr_t printer_progress_handler;

extern start_print_pkt_t PRN_PKT_START;

uint8_t gbprinter_detect(uint8_t delay) BANKED;
uint8_t gbprinter_print_image(const uint8_t * image, uint8_t image_bank, const frame_desc_t * frame, uint8_t frame_bank) BANKED;

inline void gbprinter_set_handler(void * fn_ofs, uint8_t fn_seg) {
    printer_progress_handler.SEG = fn_seg, printer_progress_handler.OFS = fn_ofs;
}

inline void gbprinter_set_print_params(uint8_t margins, uint8_t palette, uint8_t exposure) {
    PRN_PKT_START.crc = ((PRN_CMD_PRINT + 0x04u + 0x01u) + (PRN_PKT_START.margins = margins) + (PRN_PKT_START.palette = palette) + (PRN_PKT_START.exposure = exposure));
}

#endif