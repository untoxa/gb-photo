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
#include "fade.h"
#include "load_save.h"
#include "flip.h"

#include "misc_assets.h"

#include "protected.h"

BANKREF(module_protected)
BANKREF(module_sysmessages)

static const cam_magic_struct_t block_magic = {
    .magic = {'M', 'a', 'g', 'i', 'c'},
    .crc = 0
};

void protected_pack(uint8_t * v) BANKED {
    uint8_t i, elem;
    VECTOR_ITERATE(v, i, elem) {
        protected_modify_slot(elem, i);
    }
}

uint16_t protected_scale_line_part(const void * ptr) NAKED {
    ptr;
#if defined(NINTENDO)
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
        ld h, d
        ld l, e
        ld de, #16

        .rept 3
            .SCALE b,c
            add hl, de
        .endm
        .SCALE b,c

        ret
    __endasm;
#elif defined(SEGA)
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
        ld a, (hl)
        inc hl
        .SHIFT_6_2 regl
        ld a, (hl)
        dec hl
        .SHIFT_6_2 regh
.endm
        ld de, #16

        .rept 3
            .SCALE b,c
            add hl, de
        .endm
        .SCALE b,c

        ld d, b
        ld e, c
        ret
    __endasm;
#endif
}

void protected_generate_thumbnail(uint8_t slot) BANKED {
    CAMERA_SWITCH_RAM((slot >> 1) + 1);
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

void protected_lastseen_to_slot(uint8_t slot, camera_flip_e flip) BANKED {
    uint8_t * sour = get_flipped_last_seen_image(flip, true);
    CAMERA_SWITCH_RAM((slot >> 1) + 1);
    memcpy((slot & 1) ? image_second : image_first, sour, CAMERA_IMAGE_SIZE);
}

static uint8_t meta_offsets[] = { 8, 2, 2, 2, 10, 2, 2, 2, 10, 2, 2, 2, 10, 2, 2, 2 };

static inline uint8_t * thumbnail_last_row(uint8_t slot) {
    return ((slot & 1) ? image_second_thumbnail : image_first_thumbnail) + ((CAMERA_THUMB_TILE_HEIGHT - 1) * CAMERA_THUMB_TILE_WIDTH * 16);
}

uint8_t protected_metadata_read(uint8_t slot, uint8_t * dest, uint8_t size) BANKED {
    if (!size) return FALSE;
    CAMERA_SWITCH_RAM((slot >> 1) + 1);
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
    CAMERA_SWITCH_RAM((slot >> 1) + 1);
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

void protected_owner_info_write(void) BANKED {
    CAMERA_SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    cam_owner_data.magic = block_magic;
    cam_owner_data.magic.crc = protected_checksum((uint8_t *)&cam_owner_data.user_info, sizeof(cam_owner_data.user_info));
    cam_owner_data_echo = cam_owner_data;
}

static inline cam_image_metadata_block_t * cam_image_metadata(uint8_t slot) {
    return ((slot & 1) ? &image_second_meta : &image_first_meta);
}
static inline cam_image_metadata_block_t * cam_image_metadata_echo(uint8_t slot) {
    return ((slot & 1) ? &image_second_meta_echo : &image_first_meta_echo);
}

void protected_image_owner_write(uint8_t slot) BANKED {
    cam_owner_data_t owner_info;
    // get owner info
    CAMERA_SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    owner_info = cam_owner_data.user_info;
    // set image metadata
    CAMERA_SWITCH_RAM((slot >> 1) + 1);
    cam_image_metadata_block_t * data = cam_image_metadata(slot);
    data->user_info = owner_info;
    memset(&(data->meta), 0, sizeof(data->meta));
    data->magic = block_magic;
    data->magic.crc = protected_checksum((uint8_t *)data, sizeof(cam_image_metadata_block_t) - sizeof(cam_magic_struct_t));
    // write image metadata echo
    *cam_image_metadata_echo(slot) = *data;
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
    CAMERA_SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    if (prot_album_crc.crc != protected_checksum(&prot_album, ALBUM_LENGTH)) {
        memset(&prot_album, 0, ALBUM_LENGTH);
        prot_album_crc = block_magic;
        prot_album_crc.crc = protected_checksum(&prot_album, ALBUM_LENGTH);
        memcpy(&prot_album_echo, &prot_album, PROTECTED_BLOCK_SIZE(ALBUM_LENGTH));
        protected_status |= PROTECTED_REPAIR_ALBUM;
    }
    if (prot_vector_crc.crc != protected_checksum(&prot_vector, VECTOR_LENGTH)) {
        for (uint8_t * ptr = &prot_vector, i = 0; (i != VECTOR_LENGTH); *ptr++ = i++);
        prot_vector_crc = block_magic;
        prot_vector_crc.crc = protected_checksum(&prot_vector, VECTOR_LENGTH);
        memcpy(&prot_vector_echo, &prot_vector, PROTECTED_BLOCK_SIZE(VECTOR_LENGTH));
        protected_status |= PROTECTED_REPAIR_VECTOR;
    }
    CAMERA_SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    if (prot_owner_crc.crc != protected_checksum(&prot_owner, OWNER_LENGTH)) {
        memset(&prot_owner, 0, OWNER_LENGTH);
        prot_owner_crc = block_magic;
        prot_owner_crc.crc = protected_checksum(&prot_owner, OWNER_LENGTH);
        memcpy(&prot_owner_echo, &prot_owner, PROTECTED_BLOCK_SIZE(OWNER_LENGTH));
        protected_status |= PROTECTED_REPAIR_OWNER;
    }
    for (uint8_t i = 1; i != 16; i++) {
        CAMERA_SWITCH_RAM(i);
        if (image0_owner_crc.crc != protected_checksum(&image0_owner, IMAGE0_OWNER_LENGTH)) {
            memset(&image0_owner, 0, IMAGE0_OWNER_LENGTH);
            image0_owner_crc = block_magic;
            image0_owner_crc.crc = protected_checksum(&image0_owner, IMAGE0_OWNER_LENGTH);
            memcpy(&image0_owner_echo, &image0_owner, PROTECTED_BLOCK_SIZE(IMAGE0_OWNER_LENGTH));
            protected_status |= PROTECTED_REPAIR_META;
        }
        if (image1_owner_crc.crc != protected_checksum(&image1_owner, IMAGE1_OWNER_LENGTH)) {
            memset(&image1_owner, 0, IMAGE1_OWNER_LENGTH);
            image1_owner_crc = block_magic;
            image1_owner_crc.crc = protected_checksum(&image1_owner, IMAGE1_OWNER_LENGTH);
            memcpy(&image1_owner_echo, &image1_owner, PROTECTED_BLOCK_SIZE(IMAGE1_OWNER_LENGTH));
            protected_status |= PROTECTED_REPAIR_META;
        }
    }
    if (protected_status != PROTECTED_CORRECT) {
        CAMERA_SWITCH_RAM(CALIBRATION_BANK);
        memcpy(&calibration, default_calibration, sizeof(default_calibration));
        CAMERA_SWITCH_RAM(CALIBRATION_ECHO_BANK);
        memcpy(&calibration_echo, default_calibration, sizeof(default_calibration));
        protected_status |= PROTECTED_REPAIR_CAL;
    }
    CAMERA_SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
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
    screen_clear_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);

    uint8_t y = 0;

    menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "Save file errors were detected.");
    if (protected_status != PROTECTED_CORRECT) {
        y++;
        menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "Fixing checksums to prevent wiping");
        menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, "data by the original camera ROM.");
        y++;
        for (uint8_t i = protected_status, * const *ptr = repair_messages; (i); i >>= 1, ptr++) {
            if (i & 0x01) menu_text_out(0, y++, 0, WHITE_ON_BLACK, ITEM_DEFAULT, *ptr);
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