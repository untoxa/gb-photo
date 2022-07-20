#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include "musicmanager.h"
#include "joy.h"
#include "gbcamera.h"
#include "screen.h"
#include "states.h"
#include "gbprinter.h"
#include "linkcable.h"
#include "remote.h"
#include "vector.h"
#include "load_save.h"

#include "state_camera.h"
#include "state_gallery.h"

#include "misc_assets.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_main.h"
#include "menu_yesno.h"
#include "menu_debug.h"
#include "menu_settings.h"

// frames
#include "print_frames.h"

#if (DEBUG_ENABLED==1)
    #define MENUITEM_DEBUG &GalleryMenuItemDebug
    #define GALLERYMENU_HEIGHT 10
#else
    #define MENUITEM_DEBUG NULL
    #define MAINMENU_HEIGHT 9
#endif


BANKREF(state_gallery)

VECTOR_DECLARE(used_slots, uint8_t, CAMERA_MAX_IMAGE_SLOTS);
VECTOR_DECLARE(free_slots, uint8_t, CAMERA_MAX_IMAGE_SLOTS);

void gallery_toss_images() {
    VECTOR_CLEAR(used_slots), VECTOR_CLEAR(free_slots);
    for (uint8_t i = 0; i < CAMERA_MAX_IMAGE_SLOTS; i++) {
        uint8_t slot = cam_game_data.imageslots[i];
        if (slot == CAMERA_IMAGE_DELETED) VECTOR_ADD(free_slots, i); else VECTOR_ADD(used_slots, i);
    }
}

uint8_t gallery_show_picture(uint8_t image_no) {
    wait_vbl_done();
    screen_clear_rect(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, SOLID_BLACK);

    if (!VECTOR_LEN(used_slots)) return FALSE;
    uint8_t image_index = image_no;
    if (image_index >= VECTOR_LEN(used_slots)) image_index = VECTOR_LEN(used_slots) - 1;
    image_index = VECTOR_GET(used_slots, image_index);

    SWITCH_RAM((image_index >> 1) + 1);
    screen_load_image(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, ((image_index & 1) ? image_second : image_first));

    wait_vbl_done();
    screen_restore_rect(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
    return TRUE;
}

uint8_t gallery_print_picture(uint8_t image_no, uint8_t frame_no) {
    if (!VECTOR_LEN(used_slots)) return FALSE;
    uint8_t image_index = image_no;
    if (image_index >= VECTOR_LEN(used_slots)) image_index = VECTOR_LEN(used_slots) - 1;
    image_index = VECTOR_GET(used_slots, image_index);
    if (gbprinter_detect(10) == PRN_STATUS_OK) {
        gbprinter_print_image(((image_index & 1) ? image_second : image_first), (image_index >> 1) + 1, print_frames + frame_no, BANK(print_frames));
        return TRUE;
    }
    return FALSE;
}

uint8_t gallery_transfer_picture(uint8_t image_no) {
    if (!VECTOR_LEN(used_slots)) return FALSE;
    uint8_t image_index = image_no;
    if (image_index >= VECTOR_LEN(used_slots)) image_index = VECTOR_LEN(used_slots) - 1;
    image_index = VECTOR_GET(used_slots, image_index);
    linkcable_transfer_image(((image_index & 1) ? image_second : image_first), (image_index >> 1) + 1);
    return TRUE;
}

inline void show_progressbar(uint8_t x, uint8_t value, uint8_t size) {
    misc_render_progressbar(value, size, text_buffer);
    menu_text_out(x, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, text_buffer);
}

static uint8_t onPrinterProgress() BANKED {
    // printer progress callback handler
    show_progressbar(0, printer_completion, PRN_MAX_PROGRESS);
    return 0;
}

uint8_t onHelpGalleryMenu(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onTranslateSubResultGalleryMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t GalleryMenuItemInfo = {
    .prev = NULL,                           .next = &GalleryMenuItemPrint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " Info",
    .helpcontext = " View image metadata",
    .onPaint = NULL,
    .result = MENU_RESULT_CLOSE
};
const menu_item_t GalleryMenuItemPrint = {
    .prev = &GalleryMenuItemInfo,           .next = &GalleryMenuItemPrintAll,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " Print",
    .helpcontext = " Print current image",
    .onPaint = NULL,
    .result = ACTION_PRINT_IMAGE
};
const menu_item_t GalleryMenuItemPrintAll = {
    .prev = &GalleryMenuItemPrint,          .next = &GalleryMenuItemTransfer,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 8,
    .caption = " Print all",
    .helpcontext = " Print the whole gallery",
    .onPaint = NULL,
    .result = ACTION_PRINT_GALLERY
};
const menu_item_t GalleryMenuItemTransfer = {
    .prev = &GalleryMenuItemPrintAll,           .next = &GalleryMenuItemTransferAll,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 4, .width = 8,
    .caption = " Transfer",
    .helpcontext = " Link cable transfer",
    .onPaint = NULL,
    .result = ACTION_TRANSFER_IMAGE
};
const menu_item_t GalleryMenuItemTransferAll = {
    .prev = &GalleryMenuItemTransfer,           .next = &GalleryMenuItemDelete,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 5, .width = 8,
    .caption = " Transfer all",
    .helpcontext = " Transfer the whole gallery",
    .onPaint = NULL,
    .result = ACTION_TRANSFER_GALLERY
};
const menu_item_t GalleryMenuItemDelete = {
    .prev = &GalleryMenuItemTransferAll,        .next = &GalleryMenuItemDeleteAll,
    .sub = &YesNoMenu, .sub_params = "Are you sure?",
    .ofs_x = 1, .ofs_y = 6, .width = 8,
    .caption = " Delete",
    .helpcontext = " Delete current image",
    .onPaint = NULL,
    .result = ACTION_ERASE_IMAGE
};
const menu_item_t GalleryMenuItemDeleteAll = {
    .prev = &GalleryMenuItemDelete,         .next = MENUITEM_DEBUG,
    .sub = &YesNoMenu, .sub_params = "Delete all images?",
    .ofs_x = 1, .ofs_y = 7, .width = 8,
    .caption = " Delete all",
    .helpcontext = " Erase the whole gallery",
    .onPaint = NULL,
    .result = ACTION_ERASE_GALLERY
};
#if (DEBUG_ENABLED==1)
const menu_item_t GalleryMenuItemDebug = {
    .prev = &GalleryMenuItemDeleteAll,     .next = NULL,
    .sub = &DebugMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 8, .width = 8,
    .caption = " Debug",
    .helpcontext = " Show debug info",
    .onPaint = NULL,
    .result = MENU_RESULT_CLOSE
};
#endif
const menu_t GalleryMenu = {
    .x = 1, .y = 3, .width = 10, .height = GALLERYMENU_HEIGHT,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &GalleryMenuItemInfo,
    .onShow = NULL, .onHelpContext = onHelpGalleryMenu,
    .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultGalleryMenu
};

uint8_t onTranslateSubResultGalleryMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    if (menu == &YesNoMenu) {
        return (value == MENU_RESULT_YES) ? self->result : ACTION_NONE;
    }
    return value;
}
uint8_t onHelpGalleryMenu(const struct menu_t * menu, const struct menu_item_t * selection) {
    menu;
    // we draw help context here
    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, selection->helpcontext);
    return 0;
}

static void refresh_screen() {
    screen_clear_rect(0, 0, 20, 18, SOLID_BLACK);
    menu_text_out(0, 0, 20, SOLID_BLACK, " Gallery view");
    gallery_show_picture(OPTION(gallery_picture_idx));

    menu_text_out(0, 17, HELP_CONTEXT_WIDTH, SOLID_BLACK, " " ICON_START "/" ICON_SELECT " for Menus");
    sprintf(text_buffer, "%hd/%hd", (uint8_t)images_taken(), (uint8_t)images_total());
    menu_text_out(HELP_CONTEXT_WIDTH, 17, IMAGE_SLOTS_USED_WIDTH, SOLID_BLACK, text_buffer);
}

uint8_t INIT_state_gallery() BANKED {
    gallery_toss_images();
    return 0;
}

uint8_t ENTER_state_gallery() BANKED {
    refresh_screen();
    gbprinter_set_handler(onPrinterProgress, BANK(state_gallery));
    JOYPAD_RESET();
    return 0;
}

uint8_t UPDATE_state_gallery() BANKED {
    static uint8_t menu_result;
    PROCESS_INPUT();
    if (KEY_PRESSED(J_UP) || KEY_PRESSED(J_RIGHT)) {
        // next image
        if (++OPTION(gallery_picture_idx) == VECTOR_LEN(used_slots)) OPTION(gallery_picture_idx) = 0;
        gallery_show_picture(OPTION(gallery_picture_idx));
        save_camera_state();
    } else if (KEY_PRESSED(J_DOWN) || KEY_PRESSED(J_LEFT)) {
        // previous image
        if (OPTION(gallery_picture_idx)) --OPTION(gallery_picture_idx); else OPTION(gallery_picture_idx) = VECTOR_LEN(used_slots) - 1;
        gallery_show_picture(OPTION(gallery_picture_idx));
        save_camera_state();
    } else if (KEY_PRESSED(J_A)) {
        // switch to thumbnail view
        if (VECTOR_LEN(used_slots) != 0) {
            CHANGE_STATE(state_thumbnails);
            return FALSE;
        }
    } else if (KEY_PRESSED(J_SELECT)) {
        switch (menu_result = menu_execute(&GalleryMenu, NULL, NULL)) {
            case ACTION_ERASE_GALLERY:
                // TODO: erase image library
                music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
                break;
            case ACTION_ERASE_IMAGE:
                // TODO: erase image
                music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
                break;
            case ACTION_DISPLAY_INFO:
                // TODO: display image info
                music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
                break;
            case ACTION_TRANSFER_IMAGE:
                remote_activate(REMOTE_DISABLED);
                if (!gallery_transfer_picture(OPTION(gallery_picture_idx))) {
                    music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                }
                remote_activate(REMOTE_ENABLED);
                JOYPAD_RESET();
                break;
            case ACTION_TRANSFER_GALLERY:
                remote_activate(REMOTE_DISABLED);
                for (uint8_t i = 0; i != VECTOR_LEN(used_slots); i++) {
                    show_progressbar(0, i, VECTOR_LEN(used_slots));
                    if (!gallery_transfer_picture(OPTION(gallery_picture_idx))) {
                        music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                        break;
                    }
                }
                remote_activate(REMOTE_ENABLED);
                JOYPAD_RESET();
                break;
            case ACTION_PRINT_IMAGE:
                remote_activate(REMOTE_DISABLED);
                if (!gallery_print_picture(OPTION(gallery_picture_idx), OPTION(print_frame_idx))) {
                    music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                }
                remote_activate(REMOTE_ENABLED);
                JOYPAD_RESET();
                break;
            case ACTION_PRINT_GALLERY:
                remote_activate(REMOTE_DISABLED);
                for (uint8_t i = 0; i != VECTOR_LEN(used_slots); i++) {
                    if (!gallery_print_picture(i, OPTION(print_frame_idx))) {
                        music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                        break;
                    }
                }
                remote_activate(REMOTE_ENABLED);
                JOYPAD_RESET();
                break;
            default:
                // error, must not get here
                music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                break;
        }
        refresh_screen();
    } else if (KEY_PRESSED(J_START)) {
        // run Main Menu
        if (!menu_main_execute()) refresh_screen();
    }
    return TRUE;
}

uint8_t LEAVE_state_gallery() BANKED {
    gbprinter_set_handler(NULL, 0);
    return 0;
}
