/*-------------------------------------------------------------------------*/
/* PFF - Low level disk control module for the Game Boy   (C)Toxa, 2023    */
/* Based on the AVR example (C)ChaN, 2014                                  */
/*-------------------------------------------------------------------------*/

#pragma bank 255

#include <gbdk/platform.h>
#include <string.h>

#include "diskio.h"

/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#define CS_LOW()    (MMCSD_IO_REG = (MMCSD_IO_REG_SHADOW &= ~MMCSD_IO_CS))      /* Set CS low */
#define CS_HIGH()   (MMCSD_IO_REG = (MMCSD_IO_REG_SHADOW |= MMCSD_IO_CS))       /* Set CS high */
#define IS_CS_LOW   !(MMCSD_IO_REG_SHADOW & MMCSD_IO_CS)                        /* Test if CS is low */

#define dly_100us() delay(1);        /* usi.S: Delay 100 microseconds */
#define rcv_spi() xmit_spi(0xff)     /* usi.S: Send a 0xFF to the MMC and get the received byte */

volatile uint8_t AT(0x0000) MMCSD_ENABLE_REG; // 0x0B
#define MMCSD_ON        0x0B
#define MMCSD_OFF       0x00

volatile uint8_t AT(0xA000) MMCSD_IO_REG;
#define MMCSD_IO_CS     0b00000001
#define MMCSD_IO_CLK    0b00000010
#define MMCSD_IO_OUT    0b00000100
#define MMCSD_IO_IN     0b00001000
#define MMCSD_IO_B_CS   0
#define MMCSD_IO_B_CLK  1
#define MMCSD_IO_B_OUT  2
#define MMCSD_IO_B_IN   3

static uint8_t MMCSD_IO_REG_SHADOW = MMCSD_IO_CS;

inline void init_spi (void) {
    MMCSD_ENABLE_REG = MMCSD_ON;
}

uint8_t xmit_spi(int8_t spi_data) NAKED {
    spi_data;
    __asm
        ld b, a

        ld hl, #_MMCSD_IO_REG

        ld a, (_MMCSD_IO_REG_SHADOW)
        and #MMCSD_IO_CS
        ld c, a                 ; c = CS

        ld d, #MMCSD_IO_CLK

        .rept 8
            xor a
            sla b
            rla                 ; MMCSD_IO_B_OUT = data
            rlca                ; MMCSD_IO_B_CLK low
            rlca
            or c                ; set CS
            ld (hl), a

            or d                ; MMCSD_IO_B_CLK high

            ld (hl), a
            ld e, (hl)

            rrc b
            swap e
            rlc e
            rl b

            xor d               ; MMCSD_IO_B_CLK low
            ld (hl), a
        .endm

        ld (_MMCSD_IO_REG_SHADOW), a

        ld a, b
        ret
    __endasm;
}

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0    (0x40u + 0u)    /* GO_IDLE_STATE */
#define CMD1    (0x40u + 1u)    /* SEND_OP_COND (MMC) */
#define ACMD41  (0xC0u + 41u)   /* SEND_OP_COND (SDC) */
#define CMD8    (0x40u + 8u)    /* SEND_IF_COND */
#define CMD16   (0x40u + 16u)   /* SET_BLOCKLEN */
#define CMD17   (0x40u + 17u)   /* READ_SINGLE_BLOCK */
#define CMD24   (0x40u + 24u)   /* WRITE_BLOCK */
#define CMD55   (0x40u + 55u)   /* APP_CMD */
#define CMD58   (0x40u + 58u)   /* READ_OCR */


/* Card type flags (CardType) */
#define CT_NONE             0x00
#define CT_MMC              0x01    /* MMC version 3 */
#define CT_SD1              0x02    /* SD version 1 */
#define CT_SD2              0x04    /* SD version 2+ */
#define CT_BLOCK            0x08    /* Block addressing */


static uint8_t CardType = CT_NONE;


/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static uint8_t send_cmd (
    uint8_t cmd,        /* 1st byte (Start + Index) */
    uint32_t arg        /* Argument (32 bits) */
)
{
    uint8_t n, res;

    if (cmd & 0x80) {   /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        res = send_cmd(CMD55, 0);
        if (res > 1) return res;
    }

    /* Select the card */
    CS_HIGH();
    rcv_spi();
    CS_LOW();
    rcv_spi();

    /* Send a command packet */
    xmit_spi(cmd);                      /* Start + Command index */
    xmit_spi((uint8_t)(arg >> 24));     /* Argument[31..24] */
    xmit_spi((uint8_t)(arg >> 16));     /* Argument[23..16] */
    xmit_spi((uint8_t)(arg >> 8));      /* Argument[15..8] */
    xmit_spi((uint8_t)arg);             /* Argument[7..0] */
    n = 0x01;                           /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;          /* Valid CRC for CMD0(0) */
    if (cmd == CMD8) n = 0x87;          /* Valid CRC for CMD8(0x1AA) */
    xmit_spi(n);

    /* Receive a command response */
    n = 10;                             /* Wait for a valid response in timeout of 10 attempts */
    do {
        res = rcv_spi();
    } while ((res & 0x80) && --n);

    return res;         /* Return with the response value */
}




/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

uint32_t CachedSector = 0xffffffff;
uint8_t SectorCache[512];

DSTATUS disk_initialize (void) BANKED
{
    uint8_t n, cmd, ty, ocr[4];
    uint16_t tmr;

    CachedSector = 0xffffffff;

#if PF_USE_WRITE
    if (CardType != 0 && IS_CS_LOW) disk_writep(0, 0);  /* Finalize write process if it is in progress */
#endif
    init_spi();     /* Initialize ports to control MMC */
    CS_HIGH();
    for (n = 10; n; n--) rcv_spi(); /* 80 dummy clocks with CS=H */

    ty = 0;
    if (send_cmd(CMD0, 0) == 1) {           /* GO_IDLE_STATE */
        if (send_cmd(CMD8, 0x1AA) == 1) {   /* SDv2 */
            for (n = 0; n < 4; n++) ocr[n] = rcv_spi();     /* Get trailing return value of R7 resp */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {         /* The card can work at vdd range of 2.7-3.6V */
                for (tmr = 10000; tmr && send_cmd(ACMD41, 1UL << 30); tmr--) dly_100us();   /* Wait for leaving idle state (ACMD41 with HCS bit) */
                if (tmr && send_cmd(CMD58, 0) == 0) {       /* Check CCS bit in the OCR */
                    for (n = 0; n < 4; n++) ocr[n] = rcv_spi();
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* SDv2 (HC or SC) */
                }
            }
        } else {                            /* SDv1 or MMCv3 */
            if (send_cmd(ACMD41, 0) <= 1)   {
                ty = CT_SD1; cmd = ACMD41;  /* SDv1 */
            } else {
                ty = CT_MMC; cmd = CMD1;    /* MMCv3 */
            }
            for (tmr = 10000; tmr && send_cmd(cmd, 0); tmr--) dly_100us();  /* Wait for leaving idle state */
            if (!tmr || send_cmd(CMD16, 512) != 0) {    /* Set R/W block length to 512 */
                ty = 0;
            }
        }
    }
    CardType = ty;
    CS_HIGH();
    rcv_spi();

    return ty ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
    uint8_t *buff,      /* Pointer to the read buffer (NULL:Forward to the stream) */
    uint32_t sector,    /* Sector number (LBA) */
    uint16_t offset,    /* Byte offset to read from (0..511) */
    uint16_t count      /* Number of bytes to read (ofs + cnt mus be <= 512) */
) BANKED
{
    DRESULT res = RES_OK;
    uint8_t rc;

    if (CachedSector != sector) {

        if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert to byte address if needed */

        res = RES_ERROR;
        if (send_cmd(CMD17, sector) == 0) { /* READ_SINGLE_BLOCK */

            uint16_t bc = 40000;    /* Time counter */
            do {                    /* Wait for data block */
                rc = rcv_spi();
            } while (rc == 0xFF && --bc);

            if (rc == 0xFE) {   /* A data block arrived */

                uint8_t * ptr = SectorCache;

                for (bc = 512; (bc); bc--)
                    *ptr++ = rcv_spi();

                /* Skip trailing bytes in the sector and block CRC */
                rcv_spi();
                rcv_spi();

                res = RES_OK;
            }
            CachedSector = sector;
        }

        CS_HIGH();
        rcv_spi();
    }

    if (res == RES_OK) memcpy(buff, SectorCache + offset, count);

    return res;
}



/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/

#if PF_USE_WRITE
DRESULT disk_writep (
    const uint8_t *buff,    /* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
    uint32_t sc             /* Number of bytes to send, Sector number (LBA) or zero */
) BANKED
{
    DRESULT res;
    uint16_t bc;
    static uint16_t wc; /* Sector write counter */

    CachedSector = 0xffffffff;

    res = RES_ERROR;

    if (buff) {     /* Send data bytes */
        bc = sc;
        while (bc && wc) {      /* Send data bytes to the card */
            xmit_spi(*buff++);
            wc--; bc--;
        }
        res = RES_OK;
    } else {
        if (sc) {   /* Initiate sector write process */
            if (!(CardType & CT_BLOCK)) sc *= 512;  /* Convert to byte address if needed */
            if (send_cmd(CMD24, sc) == 0) {         /* WRITE_SINGLE_BLOCK */
                xmit_spi(0xFF); xmit_spi(0xFE);     /* Data block header */
                wc = 512;                           /* Set byte counter */
                res = RES_OK;
            }
        } else {    /* Finalize sector write process */
            bc = wc + 2;
            while (bc--) xmit_spi(0);   /* Fill left bytes and CRC with zeros */
            if ((rcv_spi() & 0x1F) == 0x05) {   /* Receive data resp and wait for end of write process in timeout of 500ms */
                for (bc = 5000; rcv_spi() != 0xFF && bc; bc--) {    /* Wait for ready */
                    dly_100us();
                }
                if (bc) res = RES_OK;
            }
            CS_HIGH();
            rcv_spi();
        }
    }

    return res;
}
#endif
