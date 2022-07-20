#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "gbcamera.h"
#include "gbprinter.h"

// 0b10000011 - start, CGB double speed, internal clock
#define START_TRANSFER_FAST 0x83

static const uint8_t DATA_HEADER[] = { PRN_MAGIC_1,PRN_MAGIC_2,PRN_CMD_INIT << 4,0x00, PRN_LOW(CAMERA_IMAGE_SIZE),PRN_HIGH(CAMERA_IMAGE_SIZE) };
static const uint8_t DATA_FOOTER[] = { 0x00,0x00,0x00,0x00 };

void linkcable_send_string(const uint8_t * data, uint8_t len) {
#ifdef NINTENDO
    for( ; (len); len--) {
        SB_REG = *data++;
        SC_REG = START_TRANSFER_FAST;
        while (SC_REG & 0x80);
    }
#endif
}

void linkcable_send_block(const uint8_t * image) NAKED {
    image;
#ifdef NINTENDO
    __asm
.macro .SEND_A ?lbl
        ldh (c), a
        ld (hl), #START_TRANSFER_FAST
lbl:
        bit #7, (hl)
        jr nz, lbl
.endm
        ld b, #224      ; image of 224 tiles
        ld c, #_SB_REG
        ld hl, #_SC_REG
1$:
        .rept 15
            ld a, (de)
            inc e
            .SEND_A
        .endm
        ld a, (de)
        inc de
        .SEND_A

        dec b
        jp nz, 1$
        ret
    __endasm;
#endif
}


uint8_t linkcable_transfer_image(const uint8_t * image, uint8_t image_bank) BANKED {
    SWITCH_RAM(image_bank);
    linkcable_send_string(DATA_HEADER, sizeof(DATA_HEADER));
    linkcable_send_block(image);
    linkcable_send_string(DATA_FOOTER, sizeof(DATA_HEADER));
    return PRN_STATUS_OK;
}