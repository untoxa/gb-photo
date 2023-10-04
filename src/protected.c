#pragma bank 255

#include <gbdk/platform.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "compat.h"
#include "systemhelpers.h"
#include "gbcamera.h"
#include "vector.h"
#include "joy.h"
#include "screen.h"
#include "menus.h"
#include "fade_manager.h"
#include "load_save.h"

#include "misc_assets.h"

#include "protected.h"

BANKREF(module_protected)
BANKREF(module_sysmessages)

void protected_pack(uint8_t * v) BANKED {
    uint8_t i, elem;
    VECTOR_ITERATE(v, i, elem) {
        protected_modify_slot(elem, i);
    }
}

uint16_t protected_scale_line_part(const void * ptr) NAKED {
    ptr;
#ifdef NINTENDO
    __asm
.macro .SHIFT_6_2 reg
        rla
        rla
        rl reg
        rla
        rla
        rla
        rla
        rl reg
.endm
.macro .SCALE regh regl
        ld a, (hl+)
        .SHIFT_6_2 regl
        ld a, (hl-)
        .SHIFT_6_2 regh
.endm
.macro  .ADD_A_HL
        add l
        ld l, a
        adc h
        sub l
        ld h, a
.endm
        ld h, d
        ld l, e

        .rept 3
            .SCALE b,c
            ld a, #16
            .ADD_A_HL
        .endm
        .SCALE b,c

        ret
    __endasm;
#else
    __asm
        ld hl, #0
        ret
    __endasm;
#endif
}

void protected_generate_thumbnail(uint8_t slot) BANKED {
    SWITCH_RAM((slot >> 1) + 1);
    uint16_t * sour = (slot & 1) ? image_second : image_first;
    uint16_t * dest = (slot & 1) ? image_second_thumbnail : image_first_thumbnail;
    for (uint8_t y = 0; y != 28; y++) {
        uint16_t * s = sour + ((uint16_t)((uint8_t)(y * 4) / 8)) * (CAMERA_IMAGE_TILE_WIDTH * 8) + ((y * 4) % 8);
        uint16_t * d = dest + ((uint16_t)(y / 8)) * (CAMERA_THUMB_TILE_WIDTH * 8) + (y % 8);
        for (uint8_t x = 0; x != 4; x++) {
            *d = protected_scale_line_part(s);
            d += 8, s += (4 * 8);
        }
    }
}

uint8_t * copy_data_row(uint8_t * dest, const uint8_t * sour, uint8_t count) NAKED {
    dest; sour; count;
#ifdef NINTENDO
    __asm
        ldhl sp, #2
        ld a, (hl)

        ld h, d
        ld l, e

        ld d, a     ; d == count

1$:
        .rept 16
            ld a, (bc)
            inc bc
            ld (hl+), a
        .endm

        dec d
        jr nz, 1$

        pop hl
        inc sp
        jp (hl)
    __endasm;
#else
    __asm
        pop hl
        inc sp
        ex (sp), hl
        ret
    __endasm;
#endif
}

static uint8_t hflip_loop;
uint8_t * copy_data_row_flipped(uint8_t * dest, const uint8_t * sour, uint8_t count) NAKED {
    dest; sour; count;
#ifdef NINTENDO
    __asm
        ldhl sp, #2
        ld a, (hl)
        ld l, a
        ld h, #0
        .rept 4
            add hl, hl
        .endm
        dec hl
        dec hl
        add hl, de

1$:
        ld (#_hflip_loop), a

        .rept 8
            ld d, #>_flip_recode_table
            .rept 2
                ld a, (bc)
                inc bc
                ld e, a
                ld a, (de)
                ld (hl+), a
            .endm
            ld de, #-4
            add hl, de
        .endm

        ld a, (#_hflip_loop)
        dec a
        jp nz, 1$

        pop hl
        inc sp
        jp (hl)
    __endasm;
#else
    __asm
        pop hl
        inc sp
        ex (sp), hl
        ret
    __endasm;
#endif
}

void protected_lastseen_to_slot(uint8_t slot, bool flipped) BANKED {
    static uint8_t slot_bank, * dest, * sour;
    uint8_t buffer[CAMERA_IMAGE_TILE_WIDTH * 16];

    slot_bank = (slot >> 1) + 1;
    dest = (slot & 1) ? image_second : image_first;
    sour = last_seen;

    if (!flipped) {
        for (uint8_t i = CAMERA_IMAGE_TILE_HEIGHT; i != 0; i--) {
            SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
            copy_data_row(buffer, sour, CAMERA_IMAGE_TILE_WIDTH), sour += sizeof(buffer);
            SWITCH_RAM(slot_bank);
            copy_data_row(dest, buffer, CAMERA_IMAGE_TILE_WIDTH), dest += sizeof(buffer);
        }
        return;
    }
    sour += (CAMERA_IMAGE_TILE_HEIGHT - 1) * (CAMERA_IMAGE_TILE_WIDTH * 16);
    for (uint8_t i = CAMERA_IMAGE_TILE_HEIGHT; i != 0; i--) {
        SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
        copy_data_row_flipped(buffer, sour, CAMERA_IMAGE_TILE_WIDTH), sour -= sizeof(buffer);
        SWITCH_RAM(slot_bank);
        copy_data_row(dest, buffer, CAMERA_IMAGE_TILE_WIDTH), dest += sizeof(buffer);
    }
}

static uint8_t meta_offsets[] = { 8, 2, 2, 2, 10, 2, 2, 2, 10, 2, 2, 2, 10, 2, 2, 2 };

static inline uint8_t * thumbnail_last_row(uint8_t slot) {
    return ((slot & 1) ? image_second_thumbnail : image_first_thumbnail) + ((CAMERA_THUMB_TILE_HEIGHT - 1) * CAMERA_THUMB_TILE_WIDTH * 16);
}

uint8_t protected_metadata_read(uint8_t slot, uint8_t * dest, uint8_t size) BANKED {
    if (!size) return FALSE;
    SWITCH_RAM((slot >> 1) + 1);
    uint8_t * s = thumbnail_last_row(slot);
    for (uint8_t i = 0, sz = size, * d = dest; i < LENGTH(meta_offsets); i++) {
        s += meta_offsets[i];
        *d++ = *s;
        if (!(--sz)) return TRUE;
        *d++ = *(s + 1);
        if (!(--sz)) return TRUE;
    }
    return FALSE;
}

uint8_t protected_metadata_write(uint8_t slot, uint8_t * sour, uint8_t size) BANKED {
    if (!size) return FALSE;
    SWITCH_RAM((slot >> 1) + 1);
    uint8_t * d = thumbnail_last_row(slot);
    for (uint8_t i = 0, sz = size, * s = sour; i < LENGTH(meta_offsets); i++) {
        d += meta_offsets[i];
        *d = *s++;
        if (!(--sz)) return TRUE;
        *(d + 1) = *s++;
        if (!(--sz)) return TRUE;
    }
    return FALSE;
}

uint16_t protected_checksum(uint8_t * address, uint8_t length) {
    uint8_t bsum = CAMERA_SUM_SEED, bxor = CAMERA_XOR_SEED;
    for (uint8_t i = length; (i); i--) {
        bsum += *address;
        bxor ^= *address++;
    }
    return (uint16_t)(bxor << 8) | bsum;
}

#define PROTECTED_BLOCK_SIZE(LEN) ((LEN) + sizeof(cam_magic_struct_t))

#define ALBUM_ADDRESS 0xb000        // bank 0
#define ALBUM_LENGTH 0xd2
static void AT(ALBUM_ADDRESS) prot_album;
static cam_magic_struct_t AT(ALBUM_ADDRESS + ALBUM_LENGTH) prot_album_crc;
static void AT(ALBUM_ADDRESS + PROTECTED_BLOCK_SIZE(ALBUM_LENGTH)) prot_album_echo;

#define VECTOR_ADDRESS 0xb1b2       // bank 0
#define VECTOR_LENGTH 0x1e
static void AT(VECTOR_ADDRESS) prot_vector;
static cam_magic_struct_t AT(VECTOR_ADDRESS + VECTOR_LENGTH) prot_vector_crc;
static void AT(VECTOR_ADDRESS + PROTECTED_BLOCK_SIZE(VECTOR_LENGTH)) prot_vector_echo;

#define OWNER_ADDRESS 0xafb8        // bank 1
#define OWNER_LENGTH 0x12
static void AT(OWNER_ADDRESS) prot_owner;
static cam_magic_struct_t AT(OWNER_ADDRESS + OWNER_LENGTH) prot_owner_crc;
static void AT(OWNER_ADDRESS + PROTECTED_BLOCK_SIZE(OWNER_LENGTH)) prot_owner_echo;

#define IMAGE0_OWNER_ADDRESS 0xaf00  // bank 1 and above
#define IMAGE0_OWNER_LENGTH 0x55
static void AT(IMAGE0_OWNER_ADDRESS) image0_owner;
static cam_magic_struct_t AT(IMAGE0_OWNER_ADDRESS + IMAGE0_OWNER_LENGTH) image0_owner_crc;
static void AT(IMAGE0_OWNER_ADDRESS + PROTECTED_BLOCK_SIZE(IMAGE0_OWNER_LENGTH)) image0_owner_echo;

#define IMAGE1_OWNER_ADDRESS 0xbf00  // bank 1 and above
#define IMAGE1_OWNER_LENGTH 0x55
static void AT(IMAGE1_OWNER_ADDRESS) image1_owner;
static cam_magic_struct_t AT(IMAGE1_OWNER_ADDRESS + IMAGE1_OWNER_LENGTH) image1_owner_crc;
static void AT(IMAGE1_OWNER_ADDRESS + PROTECTED_BLOCK_SIZE(IMAGE1_OWNER_LENGTH)) image1_owner_echo;

#define CALIBRATION_BANK 2
static void AT(0xaff2) calibration;
#define CALIBRATION_ECHO_BANK 8
static void AT(0xbff2) calibration_echo;
const uint8_t default_calibration[] = {125, 125, 124, 126, 124, 125, 124, 122, 123, 121, 118, 104, 194, 60};

uint8_t protected_status = PROTECTED_CORRECT;

uint8_t INIT_module_protected(void) BANKED {
    static const cam_magic_struct_t magic = {
        .magic = {'M', 'a', 'g', 'i', 'c'},
        .crc = 0
    };
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    if (prot_album_crc.crc != protected_checksum(&prot_album, ALBUM_LENGTH)) {
        memset(&prot_album, 0, ALBUM_LENGTH);
        prot_album_crc = magic;
        prot_album_crc.crc = protected_checksum(&prot_album, ALBUM_LENGTH);
        memcpy(&prot_album_echo, &prot_album, PROTECTED_BLOCK_SIZE(ALBUM_LENGTH));
        protected_status |= PROTECTED_REPAIR_ALBUM;
    }
    if (prot_vector_crc.crc != protected_checksum(&prot_vector, VECTOR_LENGTH)) {
        for (uint8_t * ptr = &prot_vector, i = 0; (i != VECTOR_LENGTH); *ptr++ = i++);
        prot_vector_crc = magic;
        prot_vector_crc.crc = protected_checksum(&prot_vector, VECTOR_LENGTH);
        memcpy(&prot_vector_echo, &prot_vector, PROTECTED_BLOCK_SIZE(VECTOR_LENGTH));
        protected_status |= PROTECTED_REPAIR_VECTOR;
    }
    SWITCH_RAM(1);
    if (prot_owner_crc.crc != protected_checksum(&prot_owner, OWNER_LENGTH)) {
        memset(&prot_owner, 0, OWNER_LENGTH);
        prot_owner_crc = magic;
        prot_owner_crc.crc = protected_checksum(&prot_owner, OWNER_LENGTH);
        memcpy(&prot_owner_echo, &prot_owner, PROTECTED_BLOCK_SIZE(OWNER_LENGTH));
        protected_status |= PROTECTED_REPAIR_OWNER;
    }
    for (uint8_t i = 1; i != 16; i++) {
        SWITCH_RAM(i);
        if (image0_owner_crc.crc != protected_checksum(&image0_owner, IMAGE0_OWNER_LENGTH)) {
            memset(&image0_owner, 0, IMAGE0_OWNER_LENGTH);
            image0_owner_crc = magic;
            image0_owner_crc.crc = protected_checksum(&image0_owner, IMAGE0_OWNER_LENGTH);
            memcpy(&image0_owner_echo, &image0_owner, PROTECTED_BLOCK_SIZE(IMAGE0_OWNER_LENGTH));
            protected_status |= PROTECTED_REPAIR_META;
        }
        if (image1_owner_crc.crc != protected_checksum(&image1_owner, IMAGE1_OWNER_LENGTH)) {
            memset(&image1_owner, 0, IMAGE1_OWNER_LENGTH);
            image1_owner_crc = magic;
            image1_owner_crc.crc = protected_checksum(&image1_owner, IMAGE1_OWNER_LENGTH);
            memcpy(&image1_owner_echo, &image1_owner, PROTECTED_BLOCK_SIZE(IMAGE1_OWNER_LENGTH));
            protected_status |= PROTECTED_REPAIR_META;
        }
    }
    if (protected_status != PROTECTED_CORRECT) {
        SWITCH_RAM(CALIBRATION_BANK);
        memcpy(&calibration, default_calibration, sizeof(default_calibration));
        SWITCH_RAM(CALIBRATION_ECHO_BANK);
        memcpy(&calibration_echo, default_calibration, sizeof(default_calibration));
        protected_status |= PROTECTED_REPAIR_CAL;
    }
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    return 0;
}

const uint8_t * const repair_messages[] = {
    "  Reset album data...\tOK!",
    "  Undelete images...\tOK!",
    "  Reset owner info...\tOK!",
    "  Reset images meta...\tOK!",
    "  Reset calibration...\tOK!"
};

uint8_t INIT_module_sysmessages(void) BANKED {
    if ((!camera_settings_reset) && (protected_status == PROTECTED_CORRECT)) return 0;

    sync_vblank();
    vwf_set_colors(DMG_WHITE, DMG_BLACK);
    screen_clear_rect(DEVICE_SCREEN_X_OFFSET, DEVICE_SCREEN_Y_OFFSET, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);

    uint8_t y = 0;

    menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "Save file errors were detected.");
    if (protected_status != PROTECTED_CORRECT) {
        y++;
        menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "Fixing checksums to prevent wiping");
        menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "data by the original camera ROM.");
        y++;
        for (uint8_t i = protected_status, * const *ptr = repair_messages; (i); i >>= 1, ptr++) {
            menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, *ptr);
        }
    }
    if (camera_settings_reset) {
        y++;
        menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "Camera settings were not found.");
        y++;
        menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "  Reset to defaults...\tOK!");
    }
    menu_text_out(0, ++y, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "Press " ICON_START " to continue...");

    fade_in_modal();

    JOYPAD_RESET();
    do {
        PROCESS_INPUT();
        sync_vblank();
    } while (!KEY_PRESSED(J_START));

    fade_out_modal();

    return 0;
}