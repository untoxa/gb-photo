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
#include "remote.h"
#include "vector.h"

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

// frames
#include "print_frames.h"

BANKREF(state_gallery)

uint8_t gallery_picture_no;

VECTOR_DECLARE(used_slots, uint8_t, CAMERA_MAX_IMAGE_SLOTS);
VECTOR_DECLARE(free_slots, uint8_t, CAMERA_MAX_IMAGE_SLOTS);

void gallery_toss_images() {
    VECTOR_CLEAR(used_slots), VECTOR_CLEAR(free_slots);
    for (uint8_t i = 0; i < CAMERA_MAX_IMAGE_SLOTS; i++) {
        uint8_t slot = cam_game_data.imageslots[i];
        if (slot == CAMERA_IMAGE_DELETED) VECTOR_ADD(free_slots, i); else VECTOR_ADD(used_slots, i);
    }
}

uint8_t gallery_show_picture(uint8_t n) {
    wait_vbl_done();
    screen_clear_rect(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, SOLID_BLACK);

    if (!VECTOR_LEN(used_slots)) return FALSE;
    if (n >= VECTOR_LEN(used_slots)) n = VECTOR_LEN(used_slots) - 1;
    n = VECTOR_GET(used_slots, n);

    SWITCH_RAM((n >> 1) + 1);
    screen_load_image(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT, ((n & 1) ? image_second : image_first));

    wait_vbl_done();
    screen_restore_rect(IMAGE_DISPLAY_X, IMAGE_DISPLAY_Y, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT);
    return TRUE;
}

uint8_t gallery_print_picture(uint8_t image_no, uint8_t frame_no) {
    uint8_t res = FALSE;

    if (!VECTOR_LEN(used_slots)) return FALSE;
    if (image_no >= VECTOR_LEN(used_slots)) image_no = VECTOR_LEN(used_slots) - 1;
    image_no = VECTOR_GET(used_slots, image_no);

    remote_activate(REMOTE_DISABLED);
    if (gbprinter_detect(10) == STATUS_OK) {
        gbprinter_print_image(((image_no & 1) ? image_second : image_first), (image_no >> 1) + 1, print_frames + frame_no, BANK(print_frames));
        res = TRUE;
    }
    remote_activate(REMOTE_ENABLED);

    return res;
}


uint8_t onHelpGalleryMenu(const struct menu_t * menu, const struct menu_item_t * selection);
const menu_item_t FrameMenuItemNoFrame = {
    .prev = NULL,                 .next = &FrameMenuItemWild,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " No Frame",
    .helpcontext = " Print without frame",
    .onPaint = NULL,
    .result = ACTION_PRINT_FRAME0
};
const menu_item_t FrameMenuItemWild = {
    .prev = &FrameMenuItemNoFrame, .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " GB Camera",
    .helpcontext = " \"GB Camera\" frame",
    .onPaint = NULL,
    .result = ACTION_PRINT_FRAME1
};
const menu_t FramesSubMenu = {
    .x = 7, .y = 4, .width = 10, .height = 4,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &FrameMenuItemNoFrame,
    .onShow = NULL, .onHelpContext = onHelpGalleryMenu,
    .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t onTranslateSubResultGalleryMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t GalleryMenuItemThumb = {
    .prev = NULL,                   .next = &GalleryMenuItemInfo,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " Thumbnails",
    .helpcontext = " Thumbnail view",
    .onPaint = NULL,
    .result = ACTION_THUMBNAILS
};
const menu_item_t GalleryMenuItemInfo = {
    .prev = &GalleryMenuItemThumb,  .next = &GalleryMenuItemPrint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " Info",
    .helpcontext = " View image metadata",
    .onPaint = NULL,
    .result = MENU_RESULT_CLOSE
};
const menu_item_t GalleryMenuItemPrint = {
    .prev = &GalleryMenuItemInfo,   .next = &GalleryMenuItemDelete,
    .sub = &FramesSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 8,
    .caption = " Print",
    .helpcontext = " Print current image",
    .onPaint = NULL,
    .result = MENU_RESULT_NONE
};
const menu_item_t GalleryMenuItemDelete = {
    .prev = &GalleryMenuItemPrint,  .next = &GalleryMenuItemDeleteAll,
    .sub = &YesNoMenu, .sub_params = "Are you sure?",
    .ofs_x = 1, .ofs_y = 4, .width = 8,
    .caption = " Delete",
    .helpcontext = " Delete current image",
    .onPaint = NULL,
    .result = ACTION_ERASE_IMAGE
};
const menu_item_t GalleryMenuItemDeleteAll = {
    .prev = &GalleryMenuItemDelete, .next = NULL,
    .sub = &YesNoMenu, .sub_params = "Delete all images?",
    .ofs_x = 1, .ofs_y = 5, .width = 8,
    .caption = " Delete all",
    .helpcontext = " Erase whole gallery",
    .onPaint = NULL,
    .result = ACTION_ERASE_GALLERY
};
const menu_t GalleryMenu = {
    .x = 1, .y = 3, .width = 10, .height = 7,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &GalleryMenuItemThumb,
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
    gallery_show_picture(gallery_picture_no);

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
    JOYPAD_RESET();
    return 0;
}

uint8_t UPDATE_state_gallery() BANKED {
    static uint8_t menu_result;
    PROCESS_INPUT();
    if (KEY_PRESSED(J_UP) || KEY_PRESSED(J_RIGHT)) {
        // next image
        if (++gallery_picture_no == VECTOR_LEN(used_slots)) gallery_picture_no = 0;
        gallery_show_picture(gallery_picture_no);
    } else if (KEY_PRESSED(J_DOWN) || KEY_PRESSED(J_LEFT)) {
        // previous image
        if (gallery_picture_no) --gallery_picture_no; else gallery_picture_no = VECTOR_LEN(used_slots) - 1;
        gallery_show_picture(gallery_picture_no);
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
            case ACTION_PRINT_FRAME0:
            case ACTION_PRINT_FRAME1:
            case ACTION_PRINT_FRAME2:
            case ACTION_PRINT_FRAME3:
                // print image
                if (!gallery_print_picture(gallery_picture_no, (menu_result - ACTION_PRINT_FRAME0))) {
                    music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                }
                break;
            case ACTION_DISPLAY_INFO:
                // TODO: display image info
                music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
                break;
            case ACTION_THUMBNAILS:
                CHANGE_STATE(state_thumbnails);
                return FALSE;
            default:
                // error, must not get here
                music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                break;
        }
        refresh_screen();
    } else if (KEY_PRESSED(J_START)) {
        // run Main Menu
        if (!MainMenuDispatch(menu_execute(&MainMenu, NULL, NULL))) refresh_screen();
    }
    return TRUE;
}

uint8_t LEAVE_state_gallery() BANKED {
    return 0;
}
