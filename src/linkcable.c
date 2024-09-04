#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "gbcamera.h"
#include "gbprinter.h"

#include "linkcable.h"

// 0b10000011 - start, CGB double speed, internal clock
#define START_TRANSFER_FAST 0x83

static const uint8_t LNK_PKT_INIT[]  = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_INIT), PRN_LE(0), PRN_LE(0x01), PRN_LE(0) };
static const uint8_t LNK_DATA_HDR[]  = { PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_INIT << 4), PRN_LE(CAMERA_IMAGE_SIZE) };
static const uint8_t LNK_DATA_FTR[]  = { PRN_LE(0), PRN_LE(0) };

void linkcable_send_string(const uint8_t * data, uint8_t len) {
#ifdef NINTENDO
    for( ; (len); len--) {
        SB_REG = *data++;
        SC_REG = START_TRANSFER_FAST;
        while (SC_REG & 0x80);
    }
#endif
}
#define LINK_SEND_COMMAND(CMD) linkcable_send_string((const uint8_t *)&(CMD), sizeof(CMD))

void linkcable_send_block(const uint8_t * image, uint8_t bank) NONBANKED NAKED {
    image; bank;
#ifdef NINTENDO
    __asm
.macro .SIO_WAIT ?lbl
lbl:
        bit #7, (hl)
        jr nz, lbl
.endm
.macro .SIO_SEND_A
        .SIO_WAIT
        ldh (_SB_REG), a
        ld (hl), #START_TRANSFER_FAST
.endm
        ld c, a
        ldh a, (__current_bank)
        push af
        ld a, c
        ldh (__current_bank), a
        ld (_rROMB0_MBC5), a

        ld hl, #_SC_REG
        .SIO_WAIT

        ld bc, #CAMERA_IMAGE_SIZE
        inc b
        inc c
        jr  2$
1$:
        ld a, (de)
        inc de
        .SIO_SEND_A

2$:
        dec c
        jr nz, 1$
        dec b
        jr nz, 1$

        .SIO_WAIT

        pop af
        ldh (__current_bank), a
        ld (_rROMB0_MBC5), a

        ret
    __endasm;
#else
    __asm__("ret");
#endif
}

uint8_t linkcable_transfer_reset(void) BANKED {
    LINK_SEND_COMMAND(LNK_PKT_INIT);
    return PRN_STATUS_OK;
}

uint8_t linkcable_transfer_image(const uint8_t * image, uint8_t image_bank) BANKED {
    CAMERA_SWITCH_RAM(image_bank & 0x0f);
    LINK_SEND_COMMAND(LNK_DATA_HDR);
    linkcable_send_block(image, image_bank);
    LINK_SEND_COMMAND(LNK_DATA_FTR);
    return PRN_STATUS_OK;
}