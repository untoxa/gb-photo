#pragma bank 255

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <stdio.h>

#include "compat.h"
#include "musicmanager.h"
#include "joy.h"
#include "screen.h"
#include "states.h"
#include "bankdata.h"
#include "fade_manager.h"
#include "gbcamera.h"
#include "gbprinter.h"
#include "linkcable.h"
#include "remote.h"
#include "load_save.h"
#include "vector.h"
#include "protected.h"

#include "state_flasher.h"
#include "state_camera.h"
#include "state_gallery.h"

#include "misc_assets.h"
#include "cursors.h"
#include "hand_cursor.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_main.h"
#include "menu_yesno.h"

// frames
#include "print_frames.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"
#include "sound_transmit.h"
#include "sound_menu_alter.h"

// assets
#include "flash.h"
#include "flasher_folder_full.h"
#include "flasher_folder_empty.h"

BANKREF(state_flasher)

static enum brows_mode_e browse_mode = browse_mode_folders;
static uint8_t thumbnails_num_pages = 0, thumbnails_page_no = 0, current_slot = 0, cursor_anim = 0, cx = 0, cy = 0, current_slot_image;

static const metasprite_t flasher[] = {
    METASPR_ITEM(16,  8,  0,  0), METASPR_ITEM(0,  8,  1,  0), METASPR_ITEM(0,  8,  2,  0), METASPR_ITEM(0,  8,  3,  0),
    METASPR_ITEM( 8,-24,  4,  0), METASPR_ITEM(0,  8,  5,  0), METASPR_ITEM(0,  8,  6,  0), METASPR_ITEM(0,  8,  7,  0),
    METASPR_ITEM( 8,-24,  8,  0), METASPR_ITEM(0,  8,  9,  0), METASPR_ITEM(0,  8, 10,  0), METASPR_ITEM(0,  8, 11,  0),
    METASPR_ITEM( 8,-24, 12,  0), METASPR_ITEM(0,  8, 13,  0), METASPR_ITEM(0,  8, 14,  0), METASPR_ITEM(0,  8, 15,  0),
    METASPR_TERM
};

#define HAND_CURSOR_BASE_TILE (0x80 - cursors_TILE_COUNT - hand_cursor_TILE_COUNT)
static const metasprite_t hand_cursor0[] = {
    METASPR_ITEM(32, 24, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t hand_cursor1[] = {
    METASPR_ITEM(33, 24, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t hand_cursor2[] = {
    METASPR_ITEM(33, 25, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t hand_cursor3[] = {
    METASPR_ITEM(32, 25, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};
static const metasprite_t* const hand_cursor[] = {hand_cursor0, hand_cursor1, hand_cursor2, hand_cursor3};

static cam_image_slots_t AT(0x4000 + (0xB1B2 - 0xA000)) slot_game_data;

static const item_coord_t folder_coords[] = {{2, 1}, {6, 1}, {10, 1}, {14, 1}, {4, 5}, {8, 5}, {12, 5}};

static const item_coord_t thumbnail_coords[MAX_FLASHER_THUMBNAILS] = {
    {0,   9}, {4,   9}, {8,   9}, {12,   9}, {16,  9},
    {0,  13}, {4,  13}, {8,  13}, {12,  13}, {16, 13}
};

static const uint8_t * const picture_addr[] = {0x6000, 0x7000, 0x4000, 0x5000};
static const uint8_t * const thumbnail_addr[] = {0x6E00, 0x7E00, 0x4E00, 0x5E00};

bool flash_slots[MAX_FLASH_SLOTS];
VECTOR_DECLARE(flash_image_slots, uint8_t, CAMERA_MAX_IMAGE_SLOTS);

inline uint8_t slot_images_taken(void) {
    return VECTOR_LEN(flash_image_slots);
}
inline uint8_t coords_to_index(uint8_t x, uint8_t y) {
    return (thumbnails_page_no * 10) + (y * 5) + x;
}
inline uint8_t coords_to_picture_no(uint8_t x, uint8_t y) {
    uint8_t image_count = VECTOR_LEN(flash_image_slots);
    if (image_count) {
        uint8_t no = coords_to_index(x, y);
        return (no < image_count) ? no : image_count;
    } else return image_count;
}

static const uint8_t MAGIC_SAVE_VALUE[] = {'M', 'a', 'g', 'i', 'c'};

uint8_t flash_save_gallery_to_slot(uint8_t slot) {
    cart_type_e cart_type = OPTION(cart_type);
    // erase the sector and save first 8 SRAM banks
    save_sram_bank_offset = FIRST_HALF_OFS;
    save_rom_bank = slot_to_sector(slot, 0);
    if (!erase_flash(cart_type)) return FALSE;
    save_sram_bank_count = FIRST_HALF_LEN;
    if (!save_sram_banks(cart_type)) return FALSE;
    // erase the next sector and save the next 8 sram banks
    save_sram_bank_offset = SECOND_HALF_OFS;
    save_rom_bank = slot_to_sector(slot, 1);
    if (!erase_flash(cart_type)) return FALSE;
    save_sram_bank_count = SECOND_HALF_LEN;
    return save_sram_banks(cart_type);
}

uint8_t flash_erase_slot(uint8_t slot) {
    cart_type_e cart_type = OPTION(cart_type);
    save_rom_bank = slot_to_sector(slot, 0);
    if (!erase_flash(cart_type)) return FALSE;
    save_rom_bank = slot_to_sector(slot, 1);
    return erase_flash(cart_type);
}

void flasher_load_gallery_from_slot(uint8_t slot) {
    uint8_t slot_bank = slot_to_sector(slot, 0);
    for (uint8_t image_index = 0; image_index != CAMERA_MAX_IMAGE_SLOTS; image_index++) {
        SWITCH_RAM((image_index >> 1) + 1);
        banked_memcpy(((image_index & 1) ? image_second : image_first),
                      (uint8_t *)picture_addr[image_index & 0x03],
                      CAMERA_IMAGE_SIZE,
                      slot_bank + (((image_index >> 1) + 1) >> 1));
        banked_memcpy(((image_index & 1) ? image_second_thumbnail : image_first_thumbnail),
                      (uint8_t *)thumbnail_addr[image_index & 0x03],
                      CAMERA_THUMB_SIZE,
                      slot_bank + (((image_index >> 1) + 1) >> 1));
    }
    banked_memcpy(text_buffer, slot_game_data.imageslots, CAMERA_MAX_IMAGE_SLOTS, slot_bank);
    for (uint8_t i = 0; i != CAMERA_MAX_IMAGE_SLOTS; i++)
        protected_modify_slot(i, text_buffer[i]);
    gallery_toss_images();
}

uint8_t flasher_restore_image(uint8_t image_no) {
    uint8_t n_images = images_taken();
    if ((n_images < CAMERA_MAX_IMAGE_SLOTS) && (image_no < slot_images_taken())) {
        uint8_t image_index = VECTOR_GET(flash_image_slots, image_no);
        uint8_t slot_bank = slot_to_sector(current_slot, 0);
        // modify index
        uint8_t image_slot = VECTOR_POP(free_slots);
        protected_modify_slot(image_slot, n_images);
        // copy image data
        SWITCH_RAM((image_slot >> 1) + 1);
        banked_memcpy(((image_slot & 1) ? image_second : image_first),
                      (uint8_t *)picture_addr[image_index & 0x03],
                      CAMERA_IMAGE_SIZE,
                      slot_bank + (((image_index >> 1) + 1) >> 1));
        banked_memcpy(((image_slot & 1) ? image_second_thumbnail : image_first_thumbnail),
                      (uint8_t *)thumbnail_addr[image_index & 0x03],
                      CAMERA_THUMB_SIZE,
                      slot_bank + (((image_index >> 1) + 1) >> 1));
        // add slot to used list
        VECTOR_ADD(used_slots, image_slot);
        return TRUE;
    }
    return FALSE;
}

uint8_t flasher_print_picture(uint8_t image_no, uint8_t frame_no) BANKED {
    if (image_no < slot_images_taken()) {
        uint8_t image_index = VECTOR_GET(flash_image_slots, image_no);
        uint8_t slot_bank = slot_to_sector(current_slot, 0);
        if (gbprinter_detect(PRINTER_DETECT_TIMEOUT) == PRN_STATUS_OK) {
            return (gbprinter_print_image((uint8_t *)picture_addr[image_index & 0x03],
                                          slot_bank + (((image_index >> 1) + 1) >> 1),
                                          print_frames + frame_no,
                                          BANK(print_frames)) == PRN_STATUS_CANCELLED) ? FALSE : TRUE;
        }
    }
    return FALSE;
}

uint8_t flasher_transfer_picture(uint8_t image_no) BANKED {
    if (image_no < slot_images_taken()) {
        uint8_t image_index = VECTOR_GET(flash_image_slots, image_no);
        uint8_t slot_bank = slot_to_sector(current_slot, 0);
        linkcable_transfer_image((uint8_t *)picture_addr[image_index & 0x03], slot_bank + (((image_index >> 1) + 1) >> 1));
        return TRUE;
    }
    return FALSE;
}

static uint8_t onPrinterProgress(void) BANKED {
    // printer progress callback handler
    flasher_show_progressbar(0, printer_completion, PRN_MAX_PROGRESS);
    return 0;
}

typedef enum {
    idFlasherSave = 0,
    idFlasherMove,
    idFlasherLoad,
    idFlasherErase,
    idFlasherPrintSlot,
    idFlasherTransferSlot,
    idFlasherDeleteAll,
} settings_menu_e;

uint8_t onHelpFlasherMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onTranslateSubResultFlasherMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onFlasherMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t onShowFlashGalleryMenu(const menu_t * self, uint8_t * param);
uint8_t onShowImagePreviewMenu(const menu_t * self, uint8_t * param);
const menu_item_t FlasherMenuItems[] = {
    {
        .sub = &YesNoMenu, .sub_params = "Save the camera roll?",
        .ofs_x = 1, .ofs_y = 1, .width = 11,
        .id = idFlasherSave,
        .caption = " Save roll to slot",
        .helpcontext = " Save the camera roll to slot",
        .onPaint = NULL,
        .onGetProps = onFlasherMenuItemProps,
        .result = ACTION_FLASH_SAVE
    }, {
        .sub = &YesNoMenu, .sub_params = "Move the camera roll?",
        .ofs_x = 1, .ofs_y = 2, .width = 11,
        .id = idFlasherMove,
        .caption = " Move roll to slot",
        .helpcontext = " Save roll to slot and clear",
        .onPaint = NULL,
        .onGetProps = onFlasherMenuItemProps,
        .result = ACTION_FLASH_MOVE
    }, {
        .sub = &YesNoMenu, .sub_params = "Load the camera roll?",
        .ofs_x = 1, .ofs_y = 3, .width = 11,
        .id = idFlasherLoad,
        .caption = " Load roll from slot",
        .helpcontext = " Load images to camera roll",
        .onPaint = NULL,
        .onGetProps = onFlasherMenuItemProps,
        .result = ACTION_FLASH_LOAD
    }, {
        .sub = &YesNoMenu, .sub_params = "Erase the slot?",
        .ofs_x = 1, .ofs_y = 4, .width = 11,
        .id = idFlasherErase,
        .caption = " Erase slot",
        .helpcontext = " Erase all images in slot",
        .onPaint = NULL,
        .onGetProps = onFlasherMenuItemProps,
        .result = ACTION_FLASH_ERASE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 5, .width = 11,
        .id = idFlasherPrintSlot,
        .caption = " Print slot",
        .helpcontext = " Print all images from slot",
        .onPaint = NULL,
        .onGetProps = onFlasherMenuItemProps,
        .result = ACTION_PRINT_SLOT
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 6, .width = 11,
        .id = idFlasherTransferSlot,
        .caption = " Transfer slot",
        .helpcontext = " Transfer all images from slot",
        .onPaint = NULL,
        .onGetProps = onFlasherMenuItemProps,
        .result = ACTION_TRANSFER_SLOT
    }, {
        .sub = &YesNoMenu, .sub_params = "Clear the camera roll?",
        .ofs_x = 1, .ofs_y = 7, .width = 11,
        .id = idFlasherDeleteAll,
        .caption = " Clear camera roll",
        .helpcontext = " Clear the camera roll",
        .onPaint = NULL,
        .onGetProps = onFlasherMenuItemProps,
        .result = ACTION_ERASE_GALLERY
    }
};
const menu_t FlasherMenu = {
    .x = 1, .y = 4, .width = 13, .height = LENGTH(FlasherMenuItems) + 2,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = FlasherMenuItems, .last_item = LAST_ITEM(FlasherMenuItems),
    .onShow = NULL, .onHelpContext = onHelpFlasherMenu,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultFlasherMenu
};

const menu_item_t FlashGalleryMenuItems[] = {
    {
        .sub = &YesNoMenu, .sub_params = "Restore image?",
        .ofs_x = 1, .ofs_y = 1, .width = 9,
        .caption = " Restore image",
        .helpcontext = " Restore image to camera roll",
        .onPaint = NULL,
        .result = ACTION_RESTORE_IMAGE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 2, .width = 9,
        .caption = " Print image",
        .helpcontext = " Print image",
        .onPaint = NULL,
        .result = ACTION_PRINT_IMAGE
    }, {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 3, .width = 9,
        .caption = " Transfer image",
        .helpcontext = " Transfer image",
        .onPaint = NULL,
        .result = ACTION_TRANSFER_IMAGE
    }
};
const menu_t FlashGalleryMenu = {
    .x = 1, .y = 5, .width = 16, .height = MAX(LENGTH(FlashGalleryMenuItems) + 2, 6),
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = FlashGalleryMenuItems, .last_item = LAST_ITEM(FlashGalleryMenuItems),
    .onShow = onShowFlashGalleryMenu, .onHelpContext = onHelpFlasherMenu,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultFlasherMenu
};

const menu_item_t ImagePreviewItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 0, .ofs_y = 0, .width = 0,
        .caption = "",
        .helpcontext = " Press " ICON_A " or " ICON_B " to continue",
        .onPaint = NULL,
        .result = ACTION_NONE
    }
};
const menu_t ImagePreviewMenu = {
    .x = 2, .y = 2, .width = 16, .height = 14,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = ImagePreviewItems, .last_item = LAST_ITEM(ImagePreviewItems),
    .onShow = onShowImagePreviewMenu, .onHelpContext = onHelpFlasherMenu,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t onTranslateSubResultFlasherMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu;
    if (self->sub == &YesNoMenu) {
        return (value == MENU_RESULT_YES) ? self->result : ACTION_NONE;
    }
    return value;
}
uint8_t onHelpFlasherMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, HELP_CONTEXT_COLOR, ITEM_DEFAULT, selection->helpcontext);
    return 0;
}
uint8_t onFlasherMenuItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    switch ((settings_menu_e)self->id) {
        case idFlasherSave:
        case idFlasherMove:
            return (!flash_slots[current_slot]) ? ITEM_DEFAULT : ITEM_DISABLED;
        case idFlasherLoad:
        case idFlasherErase:
        case idFlasherPrintSlot:
        case idFlasherTransferSlot:
            return (flash_slots[current_slot]) ? ITEM_DEFAULT : ITEM_DISABLED;
        case idFlasherDeleteAll:
            return (images_taken()) ? ITEM_DEFAULT : ITEM_DISABLED;
        default:
            return ITEM_DEFAULT;
    }
}
uint8_t onShowFlashGalleryMenu(const menu_t * self, uint8_t * param) {
    param;
    uint8_t image_index = VECTOR_GET(flash_image_slots, current_slot_image);
    uint8_t slot_bank = slot_to_sector(current_slot, 0);
    menu_draw_frame(self);
    screen_load_thumbnail_banked(self->x + 11, self->y + 1,
                                 (uint8_t *)thumbnail_addr[image_index & 0x03],
                                 0x00,
                                 slot_bank + (((image_index >> 1) + 1) >> 1));
    screen_restore_rect(self->x + 11, self->y + 1, CAMERA_THUMB_TILE_WIDTH, CAMERA_THUMB_TILE_HEIGHT);
    return MENU_PROP_NO_FRAME;
}
uint8_t onShowImagePreviewMenu(const menu_t * self, uint8_t * param) {
    param;
    uint8_t image_index = VECTOR_GET(flash_image_slots, current_slot_image);
    uint8_t slot_bank = slot_to_sector(current_slot, 0);
    screen_load_image_banked(self->x, self->y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT,
                             (uint8_t *)picture_addr[image_index & 0x03],
                             slot_bank + (((image_index >> 1) + 1) >> 1));
    screen_restore_rect(self->x, self->y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
    return MENU_PROP_NO_FRAME;
}


void flasher_read_slots(void) {
    for (uint8_t i = 0; i != MAX_FLASH_SLOTS; i++) {
        banked_memcpy(text_buffer, slot_game_data.magic.magic, sizeof(slot_game_data.magic.magic), slot_to_sector(i, 0));
        flash_slots[i] = (memcmp(text_buffer, MAGIC_SAVE_VALUE, sizeof(slot_game_data.magic.magic)) == 0);
    }
}

void flasher_refresh_folders(void) {
    sync_vblank();
    screen_clear_rect(FLASHER_FOLDER_DISPLAY_X1, FLASHER_FOLDER_DISPLAY_Y1, FLASHER_FOLDER_DISPLAY_ROW1, 4 * 2, WHITE_ON_BLACK);
    for (uint8_t i = 0; i != MAX_FLASH_SLOTS; i++) {
        if (flash_slots[i]) {
            screen_load_image_banked(folder_coords[i].x, folder_coords[i].y, 4, 4, flasher_folder_full_tiles, BANK(flasher_folder_full));
        } else {
            screen_load_image_banked(folder_coords[i].x, folder_coords[i].y, 4, 4, flasher_folder_empty_tiles, BANK(flasher_folder_empty));
        }
    }
    screen_restore_rect(FLASHER_FOLDER_DISPLAY_X1, FLASHER_FOLDER_DISPLAY_Y1, FLASHER_FOLDER_DISPLAY_ROW1, 4);
    screen_restore_rect(FLASHER_FOLDER_DISPLAY_X2, FLASHER_FOLDER_DISPLAY_Y2, FLASHER_FOLDER_DISPLAY_ROW2, 4);
}

void flasher_toss_images(void) {
    banked_memcpy(text_buffer, slot_game_data.imageslots, CAMERA_MAX_IMAGE_SLOTS, slot_to_sector(current_slot, 0));
    memset(flash_image_slots, CAMERA_IMAGE_DELETED, sizeof(flash_image_slots));
    uint8_t elem;
    for (uint8_t i = 0; i != CAMERA_MAX_IMAGE_SLOTS; i++) {
        uint8_t order = text_buffer[i];
        if (order < CAMERA_MAX_IMAGE_SLOTS) {
            elem = VECTOR_GET(flash_image_slots, order);
            if (elem == CAMERA_IMAGE_DELETED) VECTOR_SET_DIRECT(flash_image_slots, order, i);
        }
    }
    uint8_t j = 0;
    for (uint8_t i = 0; i != CAMERA_MAX_IMAGE_SLOTS; i++) {
        elem = VECTOR_GET(flash_image_slots, i);
        if (elem < CAMERA_MAX_IMAGE_SLOTS) VECTOR_SET_DIRECT(flash_image_slots, j++, elem);
    }
    VECTOR_LEN(flash_image_slots) = j;
}

uint8_t flasher_thumbnails_diaplay(uint8_t start) {
    sync_vblank();
    uint8_t slot_bank = slot_to_sector(current_slot, 0);
    screen_clear_rect(FLASHER_THUMBS_DISPLAY_X, FLASHER_THUMBS_DISPLAY_Y, FLASHER_THUMBS_DISPLAY_WIDTH, FLASHER_THUMBS_DISPLAY_HEIGHT, WHITE_ON_BLACK);
    for (uint8_t i = start, j = 0; (i < VECTOR_LEN(flash_image_slots)) && (j != MAX_FLASHER_THUMBNAILS); i++, j++) {
        uint8_t slot = VECTOR_GET(flash_image_slots, i);
        screen_load_thumbnail_banked(thumbnail_coords[j].x, thumbnail_coords[j].y,
                                     (uint8_t *)thumbnail_addr[slot & 0x03],
                                     0xFF,
                                     slot_bank + (((slot >> 1) + 1) >> 1));
        screen_restore_rect(thumbnail_coords[j].x, thumbnail_coords[j].y, CAMERA_THUMB_TILE_WIDTH, CAMERA_THUMB_TILE_HEIGHT);
    }
    return TRUE;
}

void flasher_refresh_thumbnails(void) {
    if (flash_slots[current_slot]) {
        flasher_toss_images();
        flasher_thumbnails_diaplay(thumbnails_page_no * MAX_FLASHER_THUMBNAILS);
    } else {
        sync_vblank();
        screen_clear_rect(FLASHER_THUMBS_DISPLAY_X, FLASHER_THUMBS_DISPLAY_Y, FLASHER_THUMBS_DISPLAY_WIDTH, FLASHER_THUMBS_DISPLAY_HEIGHT, WHITE_ON_BLACK);
        VECTOR_CLEAR(flash_image_slots);
    }
    sprintf(text_buffer, " Slot: %hd Images: %hd", (uint8_t)(current_slot + 1), VECTOR_LEN(flash_image_slots));
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, WHITE_ON_BLACK, ITEM_DEFAULT, text_buffer);
    return;
}

static void refresh_usage_indicator(void) {
    sprintf(text_buffer, "%hd/%hd", (uint8_t)images_taken(), (uint8_t)images_total());
    menu_text_out(HELP_CONTEXT_WIDTH, 17, IMAGE_SLOTS_USED_WIDTH, WHITE_ON_BLACK, ITEM_DEFAULT, text_buffer);
}

static void refresh_screen(void) {
    sync_vblank();
    vwf_set_colors(DMG_WHITE, DMG_BLACK);
    screen_clear_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);
    menu_text_out(0, 0, DEVICE_SCREEN_WIDTH, WHITE_ON_BLACK, ITEM_DEFAULT, " Flash directory");
    refresh_usage_indicator();

    // folders
    flasher_refresh_folders();

    // thumbnails
    flasher_refresh_thumbnails();
}

void flasher_show_icon(void) {
    fade_out_modal();
    hide_sprites_range(0, MAX_HARDWARE_SPRITES);
    screen_clear_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);
    banked_vmemcpy(_VRAM, flash_tiles, sizeof(flash_tiles), BANK(flash));
    hide_sprites_range(move_metasprite(flasher, 0, 0, (DEVICE_SCREEN_PX_WIDTH - 32) >> 1, (DEVICE_SCREEN_PX_HEIGHT - 32) >> 1), MAX_HARDWARE_SPRITES);
    fade_in_modal();
}
void flasher_hide_icon(void) {
    fade_out_modal();
    hide_sprites_range(0, MAX_HARDWARE_SPRITES);
    refresh_screen();
    fade_in_modal();
}


uint8_t INIT_state_flasher(void) BANKED {
    return 0;
}

uint8_t ENTER_state_flasher(void) BANKED {
    browse_mode = browse_mode_folders;
    flasher_read_slots();
    refresh_screen();
    gbprinter_set_handler(onPrinterProgress, BANK(state_flasher));
    fade_in_modal();
    JOYPAD_RESET();
    return 0;
}

inline void update_mode_folders_cursor(void) {
    hide_sprites_range(move_metasprite(hand_cursor[cursor_anim], HAND_CURSOR_BASE_TILE, 0, folder_coords[current_slot].x << 3, folder_coords[current_slot].y << 3), MAX_HARDWARE_SPRITES);
}

void update_mode_folders(void) {
    if (KEY_PRESSED(J_UP)) {
        PLAY_SFX(sound_error);
    } else if (KEY_PRESSED(J_DOWN)) {
        PLAY_SFX(sound_error);
    } else if (KEY_PRESSED(J_LEFT)) {
        if (current_slot-- == 0) current_slot = LENGTH(folder_coords) - 1;
        update_mode_folders_cursor();
        PLAY_SFX(sound_menu_alter);
        thumbnails_page_no = cx = cy = 0;
        flasher_refresh_thumbnails();
    } else if (KEY_PRESSED(J_RIGHT)) {
        if (++current_slot == LENGTH(folder_coords)) current_slot = 0;
        update_mode_folders_cursor();
        PLAY_SFX(sound_menu_alter);
        thumbnails_page_no = cx = cy = 0;
        flasher_refresh_thumbnails();
    } else if (KEY_PRESSED(J_A)) {
        if (flash_slots[current_slot]) {
            browse_mode = browse_mode_thumbnails;
            thumbnails_num_pages = VECTOR_LEN(flash_image_slots) / MAX_FLASHER_THUMBNAILS;
            if (VECTOR_LEN(flash_image_slots) % MAX_FLASHER_THUMBNAILS) thumbnails_num_pages++;
            PLAY_SFX(sound_menu_alter);
        } else PLAY_SFX(sound_error);
    }
    update_mode_folders_cursor();
}

void update_mode_thumbnails(void) {
    if (KEY_PRESSED(J_UP)) {
        if (cy) --cy, PLAY_SFX(sound_menu_alter);
    } else if (KEY_PRESSED(J_DOWN)) {
        if (cy < (FLASHER_THUMBS_COUNT_Y - 1)) ++cy, PLAY_SFX(sound_menu_alter);
    } else if (KEY_PRESSED(J_LEFT)) {
        if (!cx) {
            uint8_t old_page = thumbnails_page_no;
            if (thumbnails_page_no) --thumbnails_page_no; else thumbnails_page_no = thumbnails_num_pages - 1;
            cx = FLASHER_THUMBS_COUNT_X - 1;
            if (old_page != thumbnails_page_no) {
                hide_sprites_range(0, MAX_HARDWARE_SPRITES);
                flasher_thumbnails_diaplay(thumbnails_page_no * MAX_FLASHER_THUMBNAILS);
            }
        } else --cx;
        PLAY_SFX(sound_menu_alter);
    } else if (KEY_PRESSED(J_RIGHT)) {
        if (++cx == FLASHER_THUMBS_COUNT_X) {
            uint8_t old_page = thumbnails_page_no;
            if (thumbnails_page_no < thumbnails_num_pages - 1) ++thumbnails_page_no; else thumbnails_page_no = 0;
            cx = 0;
            if (old_page != thumbnails_page_no) {
                hide_sprites_range(0, MAX_HARDWARE_SPRITES);
                flasher_thumbnails_diaplay(thumbnails_page_no * MAX_FLASHER_THUMBNAILS);
            }
        };
        PLAY_SFX(sound_menu_alter);
    } else if (KEY_PRESSED(J_A)) {
        if ((current_slot_image = coords_to_picture_no(cx, cy)) < slot_images_taken()) {
            screen_clear_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, WHITE_ON_BLACK);
            menu_execute(&ImagePreviewMenu, NULL, NULL);
            PLAY_SFX(sound_ok);
            refresh_screen();
        } else PLAY_SFX(sound_error);
    } else if (KEY_PRESSED(J_B)) {
        browse_mode = browse_mode_folders;
        PLAY_SFX(sound_menu_alter);
    }
    hide_sprites_range(move_metasprite(hand_cursor[cursor_anim], HAND_CURSOR_BASE_TILE, 0, ((cx << 2) + FLASHER_THUMBS_DISPLAY_X) << 3, ((cy << 2) + FLASHER_THUMBS_DISPLAY_Y) << 3), MAX_HARDWARE_SPRITES);
}

bool clear_camera_roll(void) {
    if (images_taken() == 0) return false;
    VECTOR_CLEAR(used_slots), VECTOR_CLEAR(free_slots);
    for (uint8_t i = CAMERA_MAX_IMAGE_SLOTS; i != 0; i--) {
        protected_modify_slot(i - 1, CAMERA_IMAGE_DELETED);
        VECTOR_ADD(free_slots, i - 1);
    }
    return true;
}

uint8_t UPDATE_state_flasher(void) BANKED {
    static uint8_t menu_result;
    static enum brows_mode_e old_browse_mode;

    PROCESS_INPUT();

    old_browse_mode = browse_mode;
    if (browse_mode == browse_mode_folders) update_mode_folders(); else update_mode_thumbnails();

    if (KEY_PRESSED(J_SELECT)) {
        if (browse_mode == browse_mode_folders) {
            switch (menu_result = menu_execute(&FlasherMenu, NULL, NULL)) {
                case ACTION_ERASE_GALLERY:
                    if (clear_camera_roll()) PLAY_SFX(sound_ok); else PLAY_SFX(sound_error);
                    refresh_screen();
                    break;
                case ACTION_FLASH_SAVE:
                case ACTION_FLASH_MOVE:
                    flasher_show_icon();
                    if (flash_save_gallery_to_slot(current_slot)) {
                        flasher_read_slots();
                        if (menu_result == ACTION_FLASH_MOVE) clear_camera_roll();
                        PLAY_SFX(sound_ok);
                    } else PLAY_SFX(sound_error);
                    flasher_hide_icon();
                    JOYPAD_RESET();
                    break;
                case ACTION_FLASH_LOAD:
                    flasher_load_gallery_from_slot(current_slot);
                    PLAY_SFX(sound_ok);
                    refresh_screen();
                    break;
                case ACTION_FLASH_ERASE:
                    flasher_show_icon();
                    if (flash_erase_slot(current_slot)) {
                        flasher_read_slots();
                        PLAY_SFX(sound_ok);
                    } else PLAY_SFX(sound_error);
                    flasher_hide_icon();
                    JOYPAD_RESET();
                    break;
                case ACTION_PRINT_SLOT:
                case ACTION_TRANSFER_SLOT: {
                    gbprinter_set_handler(NULL, 0);
                    remote_activate(REMOTE_DISABLED);
                    if (menu_result == ACTION_TRANSFER_SLOT) {
                        linkcable_transfer_reset();
                        PLAY_SFX(sound_transmit);
                    }
                    uint8_t transfer_completion = 0, image_count = slot_images_taken();
                    flasher_show_progressbar(0, 0, PRN_MAX_PROGRESS);
                    for (uint8_t i = 0; i != image_count; i++) {
                        if (!((menu_result == ACTION_TRANSFER_SLOT) ? flasher_transfer_picture(i) : flasher_print_picture(i, OPTION(print_frame_idx)))) {
                            PLAY_SFX(sound_error);
                            break;
                        }
                        uint8_t current_progress = (((uint16_t)(i + 1) * PRN_MAX_PROGRESS) / image_count);
                        if (transfer_completion != current_progress) {
                            transfer_completion = current_progress;
                            flasher_show_progressbar(0, current_progress, PRN_MAX_PROGRESS);
                        }
                    }
                    remote_activate(REMOTE_ENABLED);
                    gbprinter_set_handler(onPrinterProgress, BANK(state_flasher));
                    JOYPAD_RESET();
                    refresh_screen();
                    break;
                }
                default:
                    // unknown command or cancel
                    PLAY_SFX(sound_ok);
                    refresh_screen();
                    break;
            }
        } else {
            if ((current_slot_image = coords_to_picture_no(cx, cy)) < slot_images_taken()) {
                switch (menu_result = menu_execute(&FlashGalleryMenu, NULL, NULL)) {
                    case ACTION_RESTORE_IMAGE:
                        if (flasher_restore_image(current_slot_image)) PLAY_SFX(sound_ok); else PLAY_SFX(sound_error);
                        break;
                    case ACTION_PRINT_IMAGE: {
                        remote_activate(REMOTE_DISABLED);
                        if (!flasher_print_picture(current_slot_image, OPTION(print_frame_idx))) PLAY_SFX(sound_error);
                        remote_activate(REMOTE_ENABLED);
                        JOYPAD_RESET();
                        break;
                    }
                    case ACTION_TRANSFER_IMAGE: {
                        PLAY_SFX(sound_transmit);
                        remote_activate(REMOTE_DISABLED);
                        linkcable_transfer_reset();
                        if (!flasher_transfer_picture(current_slot_image)) PLAY_SFX(sound_error);
                        remote_activate(REMOTE_ENABLED);
                        JOYPAD_RESET();
                        break;
                    }
                    default:
                        // unknown command or cancel
                        PLAY_SFX(sound_ok);
                        break;
                }
                refresh_screen();
            } else PLAY_SFX(sound_error);
        }
    } else if (KEY_PRESSED(J_START) || ((old_browse_mode == browse_mode_folders) && (KEY_PRESSED(J_B)))) {
        // run Main Menu
        hide_sprites_range(0, MAX_HARDWARE_SPRITES);
        if (!menu_main_execute()) refresh_screen();
    }

    if ((sys_time & 0x07) == 0) cursor_anim = ++cursor_anim & 0x03;
    return TRUE;
}

uint8_t LEAVE_state_flasher(void) BANKED {
    fade_out_modal();
    gbprinter_set_handler(NULL, 0);
    hide_sprites_range(0, MAX_HARDWARE_SPRITES);
    return 0;
}
