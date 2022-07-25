#pragma bank 255

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "musicmanager.h"
#include "joy.h"
#include "gbcamera.h"
#include "screen.h"
#include "states.h"
#include "vector.h"
#include "load_save.h"
#include "fade_manager.h"

#include "state_camera.h"
#include "state_thumbnails.h"
#include "state_gallery.h"

#include "misc_assets.h"

// audio assets
#include "sound_ok.h"
#include "sound_error.h"
#include "sound_menu_alter.h"

// menus
#include "menus.h"
#include "menu_codes.h"
#include "menu_main.h"

BANKREF(state_thumbnails)

static uint8_t thumbnails_num_pages = 0, thumbnails_page_no = 0, cx = 0, cy = 0, cursor_anim = 0;

const metasprite_t gallery_cursor0[] = {
	METASPR_ITEM(16,  8, 0, 0), METASPR_ITEM(0,  24, 1, 0),
    METASPR_ITEM(24,  0, 3, 0), METASPR_ITEM(0, -24, 2, 0),
    METASPR_TERM
};
const metasprite_t gallery_cursor1[] = {
	METASPR_ITEM(17,  9, 0, 0), METASPR_ITEM(0,  22, 1, 0),
    METASPR_ITEM(22,  0, 3, 0), METASPR_ITEM(0, -22, 2, 0),
    METASPR_TERM
};
const metasprite_t gallery_cursor2[] = {
	METASPR_ITEM(18, 10, 0, 0), METASPR_ITEM(0,  20, 1, 0),
    METASPR_ITEM(20,  0, 3, 0), METASPR_ITEM(0, -20, 2, 0),
    METASPR_TERM
};
const metasprite_t * const gallery_cursor[] = {gallery_cursor0, gallery_cursor1, gallery_cursor2, gallery_cursor1};

const thumb_coord_t thumbnail_coords[MAX_PREVIEW_THUMBNAILS] = {
    {2,  1}, {6,  1}, {10,  1}, {14,  1},
    {2,  5}, {6,  5}, {10,  5}, {14,  5},
    {2,  9}, {6,  9}, {10,  9}, {14,  9},
    {2, 13}, {6, 13}, {10, 13}, {14, 13}

};

inline uint8_t coords_to_picture_no(uint8_t x, uint8_t y) {
    uint8_t no = (thumbnails_page_no << 4) | (y << 2) | x;
    return (no < CAMERA_MAX_IMAGE_SLOTS) ? no : (CAMERA_MAX_IMAGE_SLOTS - 1);
}

uint8_t tumbnails_diaplay(uint8_t start) {
    wait_vbl_done();
    screen_clear_rect(THUMBNAIL_DISPLAY_X, THUMBNAIL_DISPLAY_Y, THUMBNAIL_DISPLAY_WIDTH, THUMBNAIL_DISPLAY_HEIGHT, SOLID_BLACK);
    for (uint8_t i = start, j = 0; (i < VECTOR_LEN(used_slots)) && (j != MAX_PREVIEW_THUMBNAILS); i++, j++) {
        uint8_t itm = VECTOR_GET(used_slots, i);
        SWITCH_RAM((itm >> 1) + 1);
        screen_load_image(thumbnail_coords[j].x, thumbnail_coords[j].y, CAMERA_THUMB_TILE_WIDTH, CAMERA_THUMB_TILE_HEIGHT, ((itm & 1) ? image_second_thumbnail : image_first_thumbnail));
        wait_vbl_done();
        screen_restore_rect(thumbnail_coords[j].x, thumbnail_coords[j].y, CAMERA_THUMB_TILE_WIDTH, CAMERA_THUMB_TILE_HEIGHT);
    }
    return TRUE;
}

static void refresh_screen() {
    screen_clear_rect(0, 0, 20, 18, SOLID_BLACK);
    menu_text_out(0, 0, 20, SOLID_BLACK, " Thumbnail view");

    tumbnails_diaplay(thumbnails_page_no * MAX_PREVIEW_THUMBNAILS);

    sprintf(text_buffer, "%hd/%hd", (uint8_t)((OPTION(gallery_picture_idx) < images_taken()) ? (OPTION(gallery_picture_idx) + 1) : 0), (uint8_t)images_taken());
    menu_text_out(HELP_CONTEXT_WIDTH, 17, IMAGE_SLOTS_USED_WIDTH, SOLID_BLACK, text_buffer);
}

uint8_t INIT_state_thumbnails() BANKED {
    return 0;
}

uint8_t ENTER_state_thumbnails() BANKED {
    thumbnails_num_pages = VECTOR_LEN(used_slots) >> 4;
    if (VECTOR_LEN(used_slots) & 0x0f) thumbnails_num_pages++;

    thumbnails_page_no = (OPTION(gallery_picture_idx) >> 4);

    cx = OPTION(gallery_picture_idx) & 0x03, cy = (OPTION(gallery_picture_idx) >> 2) & 0x03;

    refresh_screen();
    fade_in_modal();
    JOYPAD_RESET();
    return 0;
}

uint8_t UPDATE_state_thumbnails() BANKED {
    PROCESS_INPUT();
    if (KEY_PRESSED(J_UP)) {
        if (cy) --cy, music_play_sfx(BANK(sound_menu_alter), sound_menu_alter, SFX_MUTE_MASK(sound_menu_alter));
    } else if (KEY_PRESSED(J_DOWN)) {
        if (cy < (THUMBS_COUNT_Y - 1)) ++cy, music_play_sfx(BANK(sound_menu_alter), sound_menu_alter, SFX_MUTE_MASK(sound_menu_alter));
    } else if (KEY_PRESSED(J_LEFT)) {
        if (!cx) {
            uint8_t old_page = thumbnails_page_no;
            if (thumbnails_page_no) --thumbnails_page_no; else thumbnails_page_no = thumbnails_num_pages - 1;
            cx = THUMBS_COUNT_X - 1;
            if (old_page != thumbnails_page_no) {
                hide_sprites_range(0, MAX_HARDWARE_SPRITES);
                tumbnails_diaplay(thumbnails_page_no * MAX_PREVIEW_THUMBNAILS);
            }
        } else --cx;
        music_play_sfx(BANK(sound_menu_alter), sound_menu_alter, SFX_MUTE_MASK(sound_menu_alter));
    } else if (KEY_PRESSED(J_RIGHT)) {
        if (++cx == THUMBS_COUNT_X) {
            uint8_t old_page = thumbnails_page_no;
            if (thumbnails_page_no < thumbnails_num_pages - 1) ++thumbnails_page_no; else thumbnails_page_no = 0;
            cx = 0;
            if (old_page != thumbnails_page_no) {
                hide_sprites_range(0, MAX_HARDWARE_SPRITES);
                tumbnails_diaplay(thumbnails_page_no * MAX_PREVIEW_THUMBNAILS);
            }
        };
        music_play_sfx(BANK(sound_menu_alter), sound_menu_alter, SFX_MUTE_MASK(sound_menu_alter));
    } else if (KEY_PRESSED(J_A)) {
        OPTION(gallery_picture_idx) = coords_to_picture_no(cx, cy);
        save_camera_state();
        music_play_sfx(BANK(sound_ok), sound_ok, SFX_MUTE_MASK(sound_ok));
        CHANGE_STATE(state_gallery);
        return 0;
    } else if (KEY_PRESSED(J_B)) {
        music_play_sfx(BANK(sound_error), sound_error, SFX_MUTE_MASK(sound_error));
        CHANGE_STATE(state_gallery);
        return 0;
    } else if (KEY_PRESSED(J_START)) {
        // run Main Menu
        hide_sprites_range(0, MAX_HARDWARE_SPRITES);
        if (!menu_main_execute()) refresh_screen();
    }
    hide_sprites_range(move_metasprite(gallery_cursor[cursor_anim], 0xfa, 0, ((cx << 2) + 2) << 3, ((cy << 2) + 1) << 3), MAX_HARDWARE_SPRITES);
    if ((sys_time & 0x07) == 0) cursor_anim = ++cursor_anim & 0x03;
    return TRUE;
}

uint8_t LEAVE_state_thumbnails() BANKED {
    fade_out_modal();
    hide_sprites_range(0, MAX_HARDWARE_SPRITES);
    return 0;
}
