#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

#include "compat.h"
#include "gbprinter.h"
#include "gbcamera.h"
#include "bankdata.h"
#include "states.h"
#include "globals.h"
#include "joy.h"
#include "screen.h"

#include "state_camera.h"

#define REINIT_SEIKO

#define START_TRANSFER 0x81
#if (CGB_FAST_TRANSFER==1)
    // 0b10000011 - start, CGB double speed, internal clock
    #define START_TRANSFER_FAST 0x83
#else
    // 0b10000001 - start, internal clock
    #define START_TRANSFER_FAST 0x81
#endif
#define PRN_BUSY_TIMEOUT        PRN_SECONDS(2)
#define PRN_COMPLETION_TIMEOUT  PRN_SECONDS(20)
#define PRN_SEIKO_RESET_TIMEOUT 10

#define PRN_FINAL_MARGIN        0x03

static const uint8_t PRN_PKT_INIT[]    = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_INIT),   PRN_LE(0), PRN_LE(0x01), PRN_LE(0) };
static const uint8_t PRN_PKT_STATUS[]  = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_STATUS), PRN_LE(0), PRN_LE(0x0F), PRN_LE(0) };
static const uint8_t PRN_PKT_EOF[]     = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_DATA),   PRN_LE(0), PRN_LE(0x04), PRN_LE(0) };
static const uint8_t PRN_PKT_CANCEL[]  = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_BREAK),  PRN_LE(0), PRN_LE(0x01), PRN_LE(0) };

start_print_pkt_t PRN_PKT_START = {
    .magic = PRN_MAGIC, .command = PRN_CMD_PRINT, .length = 4,
    .print = TRUE, .margins = 0, .palette = PRN_PALETTE_NORMAL, .exposure = PRN_EXPOSURE_DARK,
    .crc = 0, .trail = 0
};

static uint16_t printer_status;
static uint8_t printer_tile_num;

uint8_t printer_completion = 0;
far_ptr_t printer_progress_handler = {0, NULL};

#if defined(NINTENDO)
uint8_t printer_send_receive(uint8_t b) {
    SB_REG = b;
    SC_REG = (OPTION(print_fast)) ? START_TRANSFER_FAST : START_TRANSFER;
    while (SC_REG & 0x80);
    return SB_REG;
}
#elif defined(SEGA)
// DPC0 = MISO
// DPC1 = MOSI
// DPC6 = CLK
uint8_t link_exchange(uint8_t data) NAKED PRESERVES_REGS(h, l, iyh, iyl) {
    data;
    __asm
        ld e, #LINK_MASTER
        ld c, #_GG_EXT_CTL
        out (c), e
        ld c, #_GG_EXT_7BIT
        ld b, #8
1$:
        rlca
        rl e
        rl e

        res LINK_PIN_CLK, e
        out (c), e

        ld d, b
        ld b, #15
2$:     djnz 2$

        set LINK_PIN_CLK, e
        out (c), e

        ld b, #8
3$:     djnz 3$
        ld b, d

        in e, (c)
        rrca
        srl e
        rla
        djnz 1$
        ret
    __endasm;
}
uint8_t link_exchange_fast(uint8_t data) NAKED PRESERVES_REGS(h, l, iyh, iyl) {
    data;
    __asm
        ld e, #LINK_MASTER
        ld c, #_GG_EXT_CTL
        out (c), e
        ld c, #_GG_EXT_7BIT
        ld b, #8
1$:
        rlca
        rl e
        rl e

        res LINK_PIN_CLK, e
        out (c), e
        set LINK_PIN_CLK, e
        out (c), e
        in e, (c)

        rrca
        srl e
        rla
        djnz 1$
        ret
    __endasm;
}
inline uint8_t printer_send_receive(uint8_t data) {
    return (OPTION(print_fast)) ? link_exchange_fast(data) : link_exchange(data);
}

uint16_t get_vram_word(uint8_t * sour) NAKED {
    sour;
    __asm
        ex de, hl
        ld hl, #__shadow_OAM_OFF
        inc (hl)

        ld c, #_VDP_CMD
        ld a, e
        di
        out (c), e
        ei
        out (c), d

        ld c, #_VDP_DATA

        in e, (c)
        jr 1$
1$:
        nop
        in d, (c)

        ld hl, #__shadow_OAM_OFF
        dec (hl)
        ret
    __endasm;
}
void get_vram_2bpp_tile(uint8_t * dest, uint8_t * sour) NAKED {
    dest; sour;
    __asm
        ex de, hl
        ld b, h
        ld c, l
        ld hl, #__shadow_OAM_OFF
        inc (hl)

        ld a, c
        di
        out (_VDP_CMD), a
        ld a, b
        ei
        out (_VDP_CMD), a

        ex de, hl
        ld e, #8
        ld c, #_VDP_DATA
1$:
        ld b, #2
2$:
        ini
        jr nz, 2$
        nop
        in a, (_VDP_DATA)
        jr 3$
3$:
        nop
        in a, (_VDP_DATA)
        dec e
        jr nz, 1$

        ld hl, #__shadow_OAM_OFF
        dec (hl)

        ret
    __endasm;
}
#endif

uint8_t printer_send_byte(uint8_t b) {
    return (uint8_t)(printer_status = ((printer_status << 8) | printer_send_receive(b)));
}

uint8_t printer_send_command(const uint8_t *command, uint8_t length) {
    uint8_t index = 0;
    while (index++ < length) printer_send_byte(*command++);
    return ((uint8_t)(printer_status >> 8) == PRN_MAGIC_DETECT) ? (uint8_t)printer_status : PRN_STATUS_MASK_ERRORS;
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

inline void printer_init(void) {
    printer_tile_num = 0;
    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
}

inline bool printer_check_cancel(void) {
    PROCESS_INPUT();
    return (KEY_PRESSED(J_B));
}

uint8_t printer_wait(uint16_t timeout, uint8_t mask, uint8_t value) {
    uint8_t error;
    while (((error = PRINTER_SEND_COMMAND(PRN_PKT_STATUS)) & mask) != value) {
        if (printer_check_cancel()) {
            PRINTER_SEND_COMMAND(PRN_PKT_CANCEL);
            return PRN_STATUS_CANCELLED;
        }
        if (timeout-- == 0) return PRN_STATUS_MASK_ERRORS;
        if (error & PRN_STATUS_MASK_ERRORS) break;
        sync_vblank();
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
    static uint8_t error;

    // call printer progress: zero progress
    printer_completion = 0, call_far(&printer_progress_handler);

    banked_memcpy(&current_frame, frame, sizeof(current_frame), frame_bank);

    uint8_t tile_data[0x10], rows = ((current_frame.height >> 1) << 1), pkt_count = 0;

    if ((rows >> 1) == 0) return PRN_STATUS_OK;

    CAMERA_SWITCH_RAM(image_bank & 0x0f);
    img = image;

    const uint8_t * map = current_frame.map;
    printer_tile_num = 0;

    for (uint8_t y = 0; y != rows; y++) {
        for (uint8_t x = 0; x != PRN_TILE_WIDTH; x++, map++) {
            // copy frame tile if applicable
            if (current_frame.map) {
                uint8_t tileno = read_banked_ubyte(map, current_frame.map_bank);
                banked_memcpy(tile_data, current_frame.tiles + ((uint16_t)tileno << 4), sizeof(tile_data), current_frame.tiles_bank);
            } else memset(tile_data, 0, sizeof(tile_data));
            // overlay the picture tile if in range
            if ((y >= current_frame.image_y) && (y < (current_frame.image_y + CAMERA_IMAGE_TILE_HEIGHT)) &&
                (x >= current_frame.image_x) && (x < (current_frame.image_x + CAMERA_IMAGE_TILE_WIDTH))) {
                banked_memcpy(tile_data, img + ((((y - current_frame.image_y) << 4) + (x - current_frame.image_x)) << 4), sizeof(tile_data), image_bank);
            }
            // print the resulting tile
            if (printer_print_tile(tile_data)) {
                pkt_count++;
                if (printer_check_cancel()) {
                    PRINTER_SEND_COMMAND(PRN_PKT_CANCEL);
                    return PRN_STATUS_CANCELLED;
                }
            }
            if (pkt_count == 9) {
                pkt_count = 0;
                PRINTER_SEND_COMMAND(PRN_PKT_EOF);
                // setup margin if last packet
                gbprinter_set_print_params((y == (rows - 1)) ? PRN_FINAL_MARGIN : PRN_NO_MARGINS, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
                PRINTER_SEND_COMMAND(PRN_PKT_START);
                // query printer status
                if ((error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY)) & PRN_STATUS_MASK_ERRORS) return error;
                if ((error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0)) & PRN_STATUS_MASK_ERRORS) return error;
#ifdef REINIT_SEIKO
                // reinit printer (required by Seiko?)
                if (y != (rows - 1)) {
                    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
                    if (error = printer_wait(PRN_SEIKO_RESET_TIMEOUT, PRN_STATUS_MASK_ANY, PRN_STATUS_OK)) return error;
                }
#endif
                // call printer progress callback
                uint8_t current_progress = (((uint16_t)y * PRN_MAX_PROGRESS) / rows);
                if (printer_completion != current_progress) {
                    printer_completion = current_progress, call_far(&printer_progress_handler);
                }
            }
        }
    }
    if (pkt_count) {
        PRINTER_SEND_COMMAND(PRN_PKT_EOF);
        // setup printing if required
        gbprinter_set_print_params(PRN_FINAL_MARGIN, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
        PRINTER_SEND_COMMAND(PRN_PKT_START);
        // query printer status
        if ((error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY)) & PRN_STATUS_MASK_ERRORS) return error;
        if ((error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0)) & PRN_STATUS_MASK_ERRORS) return error;
        // call printer progress: indicate 100% completion
        printer_completion = PRN_MAX_PROGRESS, call_far(&printer_progress_handler);
    }
    return PRINTER_SEND_COMMAND(PRN_PKT_INIT);
}

uint8_t gbprinter_print_screen_rect(uint8_t sx, uint8_t sy, uint8_t sw, uint8_t sh, uint8_t centered) BANKED {
    static uint8_t error;

    // call printer progress: zero progress
    printer_completion = 0, call_far(&printer_progress_handler);

    uint8_t tile_data[0x10], rows = ((sh & 0x01) ? (sh + 1) : sh), pkt_count = 0, x_ofs = (centered) ? ((PRN_TILE_WIDTH - sw) >> 1) : 0;

    printer_tile_num = 0;

    for (uint8_t y = 0; y != rows; y++) {
        uint8_t * map_addr = get_bkg_xy_addr(sx, y + sy);
        for (uint8_t x = 0; x != PRN_TILE_WIDTH; x++) {
            if ((x >= x_ofs) && (x < (x_ofs + sw)) && (y < sh))  {
#if defined(NINTENDO)
                uint8_t tile = get_vram_byte(map_addr++);
                uint8_t * source = (((y + sy) > 11) || (tile > 127)) ? TILE_BANK_1 : (TILE_BANK_0 + 0x800);
                vmemcpy(tile_data, source + ((uint16_t)tile << 4), sizeof(tile_data));
#elif defined(SEGA)
                uint16_t tile = get_vram_word(map_addr) & 0x01ff;
                map_addr += 2;
                get_vram_2bpp_tile(tile_data, TILE_BANK_0 + (tile << 5));
#endif
            } else memset(tile_data, 0x00, sizeof(tile_data));
            if (printer_print_tile(tile_data)) {
                pkt_count++;
                if (printer_check_cancel()) {
                    PRINTER_SEND_COMMAND(PRN_PKT_CANCEL);
                    return PRN_STATUS_CANCELLED;
                }
            }
            if (pkt_count == 9) {
                pkt_count = 0;
                PRINTER_SEND_COMMAND(PRN_PKT_EOF);
                gbprinter_set_print_params((y == (rows - 1)) ? PRN_FINAL_MARGIN : PRN_NO_MARGINS, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
                PRINTER_SEND_COMMAND(PRN_PKT_START);
                // query printer status
                if ((error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY)) & PRN_STATUS_MASK_ERRORS) return error;
                if ((error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0)) & PRN_STATUS_MASK_ERRORS) return error;
#ifdef REINIT_SEIKO
                // reinit printer (required by Seiko?)
                if (y != (rows - 1)) {
                    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
                    if (error = printer_wait(PRN_SEIKO_RESET_TIMEOUT, PRN_STATUS_MASK_ANY, PRN_STATUS_OK)) return error;
                }
#endif
                // call printer progress callback
                uint8_t current_progress = (((uint16_t)y * PRN_MAX_PROGRESS) / rows);
                if (printer_completion != current_progress) {
                    printer_completion = current_progress, call_far(&printer_progress_handler);
                }
            }
        }
    }
    if (pkt_count) {
        PRINTER_SEND_COMMAND(PRN_PKT_EOF);
        // setup printing if required
        gbprinter_set_print_params(PRN_FINAL_MARGIN, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
        PRINTER_SEND_COMMAND(PRN_PKT_START);
        // query printer status
        if ((error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY)) & PRN_STATUS_MASK_ERRORS) return error;
        if ((error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0)) & PRN_STATUS_MASK_ERRORS) return error;
        // indicate 100% completion
        printer_completion = PRN_MAX_PROGRESS, call_far(&printer_progress_handler);
    }
    return PRINTER_SEND_COMMAND(PRN_PKT_STATUS);
}