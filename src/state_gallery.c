#pragma bank 255

#include <gbdk/platform.h>

#include "musicmanager.h"
#include "joy.h"
#include "gbcamera.h"
#include "screen.h"
#include "states.h"
#include "gbprinter.h"
#include "remote.h"

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

const menu_item_t FrameMenuItemNoFrame = {
    .prev = NULL,                 .next = &FrameMenuItemWild,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " No Frame",
    .onPaint = NULL,
    .result = ACTION_PRINT_FRAME0
};
const menu_item_t FrameMenuItemWild = {
    .prev = &FrameMenuItemNoFrame, .next = NULL,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " Wild",
    .onPaint = NULL,
    .result = ACTION_PRINT_FRAME1
};
const menu_t FramesSubMenu = {
    .x = 7, .y = 4, .width = 10, .height = 4,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &FrameMenuItemNoFrame,
    .onShow = NULL, .onTranslateKey = NULL, .onTranslateSubResult = NULL
};

uint8_t onTranslateSubResultGalleryMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
const menu_item_t GalleryMenuItemThumb = {
    .prev = NULL,                   .next = &GalleryMenuItemInfo,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 1, .width = 8,
    .caption = " Thumbnails",
    .onPaint = NULL,
    .result = ACTION_THUMBNAILS
};
const menu_item_t GalleryMenuItemInfo = {
    .prev = &GalleryMenuItemThumb,  .next = &GalleryMenuItemPrint,
    .sub = NULL, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 2, .width = 8,
    .caption = " Info",
    .onPaint = NULL,
    .result = MENU_RESULT_CLOSE
};
const menu_item_t GalleryMenuItemPrint = {
    .prev = &GalleryMenuItemInfo,   .next = &GalleryMenuItemDelete,
    .sub = &FramesSubMenu, .sub_params = NULL,
    .ofs_x = 1, .ofs_y = 3, .width = 8,
    .caption = " Print",
    .onPaint = NULL,
    .result = MENU_RESULT_NONE
};
const menu_item_t GalleryMenuItemDelete = {
    .prev = &GalleryMenuItemPrint,  .next = &GalleryMenuItemDeleteAll,
    .sub = &YesNoMenu, .sub_params = "Are you sure?",
    .ofs_x = 1, .ofs_y = 4, .width = 8,
    .caption = " Delete",
    .onPaint = NULL,
    .result = ACTION_ERASE_IMAGE
};
const menu_item_t GalleryMenuItemDeleteAll = {
    .prev = &GalleryMenuItemDelete, .next = NULL,
    .sub = &YesNoMenu, .sub_params = "Delete all images?",
    .ofs_x = 1, .ofs_y = 5, .width = 8,
    .caption = " Delete all",
    .onPaint = NULL,
    .result = ACTION_ERASE_GALLERY
};
const menu_t GalleryMenu = {
    .x = 1, .y = 3, .width = 10, .height = 7,
    .cancel_mask = J_B, .cancel_result = ACTION_NONE,
    .items = &GalleryMenuItemThumb,
    .onShow = NULL, .onTranslateKey = NULL, .onTranslateSubResult = onTranslateSubResultGalleryMenu
};

uint8_t onTranslateSubResultGalleryMenu(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    if (menu == &YesNoMenu) {
        return (value == MENU_RESULT_YES) ? self->result : ACTION_NONE;
    }
    return value;
}

static void refresh_screen() {
    screen_clear_rect(0, 0, 20, 18, SOLID_BLACK);
    screen_text_out(0, 0, "\x03\xff Gallery view");
    screen_show_picture(gallery_picture_no);
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
        if (++gallery_picture_no == CAMERA_MAX_IMAGE_SLOTS) gallery_picture_no = 0;
        screen_show_picture(gallery_picture_no);
    } else if (KEY_PRESSED(J_DOWN) || KEY_PRESSED(J_LEFT)) {
        // previous image
        if (gallery_picture_no) --gallery_picture_no; else gallery_picture_no = CAMERA_MAX_IMAGE_SLOTS - 1;
        screen_show_picture(gallery_picture_no);
    } else if (KEY_PRESSED(J_B)) {
        // switch to thumbnail view
        CHANGE_STATE(state_thumbnails);
        return 0;
    } else if (KEY_PRESSED(J_A)) {
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
                remote_activate(REMOTE_DISABLED);
                if (gbprinter_detect(10) == STATUS_OK) {
                    gbprinter_print_image(gallery_picture_no, print_frames + (menu_result - ACTION_PRINT_FRAME0), BANK(print_frames));
                } else music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
                remote_activate(REMOTE_ENABLED);
                break;
            case ACTION_DISPLAY_INFO:
                // TODO: display image info
                music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
                break;
            case ACTION_THUMBNAILS:
                CHANGE_STATE(state_thumbnails);
                return 0;
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
