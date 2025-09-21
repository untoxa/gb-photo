#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "systemhelpers.h"
#include "globals.h"
#include "joy.h"
#include "screen.h"

#include "menus.h"
#include "menu_codes.h"

#include "misc_assets.h"
#include "keyboard.h"
#include "keyboard_caps.h"
#include "cursors.h"
#include "hand_cursor.h"

#define HAND_CURSOR_BASE_TILE (0x80 - cursors_TILE_COUNT - hand_cursor_TILE_COUNT)
static const metasprite_t hand_cursor[] = {
    METASPR_ITEM(DEVICE_SPRITE_PX_OFFSET_Y, DEVICE_SPRITE_PX_OFFSET_X - 6, 0, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 1, (S_PALETTE | S_PAL(1))), METASPR_ITEM(8, -8, 2, (S_PALETTE | S_PAL(1))), METASPR_ITEM(0, 8, 3, (S_PALETTE | S_PAL(1))),
    METASPR_TERM
};

uint8_t * edit_text, edit_max_len;
uint8_t edit_pointer_x, edit_pointer_y;
uint8_t edit_lim_left, edit_lim_right, edit_lim_top, edit_lim_bottom;
bool edit_shift_state = false;

uint8_t onIdleEdit(const struct menu_t * menu, const struct menu_item_t * selection);
uint8_t onShowEdit(const struct menu_t * self, uint8_t * param);
uint8_t onTranslateKeyEdit(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value);
uint8_t onEditItemProps(const struct menu_t * menu, const struct menu_item_t * self);
uint8_t * onEditItemPaint(const struct menu_t * menu, const struct menu_item_t * self);
const menu_item_t EditItems[] = {
    {
        .sub = NULL, .sub_params = NULL,
        .ofs_x = 1, .ofs_y = 1, .width = 18,
        .caption = "%s|",
        .onPaint = onEditItemPaint,
        .onGetProps = onEditItemProps,
        .result = MENU_RESULT_NONE
    }
};
const menu_t EditMenu = {
    .x = 0, .y = 9, .width = 20, .height = 8,
    .cancel_mask = J_B, .cancel_result = MENU_RESULT_NO,
    .items = EditItems, .last_item = LAST_ITEM(EditItems),
    .onShow = onShowEdit, .onIdle = onIdleEdit,
    .onTranslateKey = onTranslateKeyEdit, .onTranslateSubResult = NULL
};
uint8_t onEditItemProps(const struct menu_t * menu, const struct menu_item_t * self) {
    menu; self;
    return ITEM_TEXT_CENTERED;
}
uint8_t * onEditItemPaint(const struct menu_t * menu, const struct menu_item_t * self) {
    menu;
    sprintf(text_buffer, self->caption, edit_text);
    return text_buffer;
}

typedef struct box_t {
    uint8_t left, top, right, bottom;
} box_t;
#define BOX(X, Y, W, H) {(X), (Y), ((X) + (W)), ((Y) + (H))}
#define BTN(X, Y) {(X), (Y), ((X) + 8), ((Y) + 9)}

static const box_t button_map[] = {
    BTN(3,  0), BTN(13,  0), BTN(23,  0), BTN(33,  0), BTN(43,  0), BTN(53,  0), BTN(63,  0), BTN(73,  0), BTN(83,  0), BTN(93,  0), BTN(103,  0), BTN(113,  0), BTN(123,  0), BTN(133,  0),
    BTN(7, 12), BTN(17, 12), BTN(27, 12), BTN(37, 12), BTN(47, 12), BTN(57, 12), BTN(67, 12), BTN(77, 12), BTN(87, 12), BTN(97, 12), BTN(107, 12), BTN(117, 12),
    BTN(0, 24), BTN(10, 24), BTN(20, 24), BTN(30, 24), BTN(40, 24), BTN(50, 24), BTN(60, 24), BTN(70, 24), BTN(80, 24), BTN(90, 24), BTN(100, 24), BTN(110, 24), BTN(120, 24),
    BTN(4, 36), BTN(14, 36), BTN(24, 36), BTN(34, 36), BTN(44, 36), BTN(54, 36), BTN(64, 36), BTN(74, 36), BTN(84, 36), BTN(94, 36), BTN(104, 36), BTN(114, 36),
    BOX(124, 36, 18, 9),                      // space
    BOX(127, 12, 12, 11), BOX(130, 24, 20, 9) // return
};
static const uint8_t lower_case[] = {
    '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0', 0xdf,  '{',  '}', 0x08,
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p', 0xfc,  '+',
    0x0f, 'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l', 0xf6,  0xe4, '#',
    '<',  'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '-',  '\\',
    ' ',
    0x0d, 0x0d
};
static const uint8_t upper_case[] = {
    '!',  '"', 0xa3,  '$',  '%',  '&',  '/',  '(',  ')',  '=',  '?',  '[',  ']', 0x08,
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '[',  '*',
    0x0f, 'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ';', 0xd6, 0xc4,
    '>',  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  ';',  ':',  '_',  '/',
    ' ',
    0x0d, 0x0d
};
uint8_t get_keyboard_input(uint8_t x, uint8_t y, bool shift) {
    for (uint8_t i = 0; i != LENGTH(button_map); ++i) {
        if ((x >= button_map[i].left) && (y >= button_map[i].top) && (x <= button_map[i].right) && (y <= button_map[i].bottom)) {
            return (shift) ? upper_case[i] : lower_case[i];
        }
    }
    return 0;
}

uint8_t onIdleEdit(const struct menu_t * menu, const struct menu_item_t * selection) {
    selection;
    static uint8_t len, letter;

    hide_sprites_range(move_metasprite(hand_cursor, HAND_CURSOR_BASE_TILE, 0, edit_pointer_x, edit_pointer_y), MAX_HARDWARE_SPRITES);

    if (KEY_PRESSED(J_A)) {
        switch (letter = get_keyboard_input(edit_pointer_x - edit_lim_left, edit_pointer_y - edit_lim_top, edit_shift_state)) {
            case 0x00:
                break;
            case 0x08:
                if (len = strlen(edit_text)) {
                    edit_text[len - 1] = 0;
                    menu_move_selection(menu, NULL, EditItems);
                }
                break;
            case 0x0d:
                return MENU_RESULT_OK;
            case 0x0f:
                if (edit_shift_state = !edit_shift_state) {
                    screen_load_image_banked(menu->x + ((menu->width - (keyboard_caps_WIDTH / keyboard_caps_TILE_W)) >> 1), menu->y + 2,
                                             keyboard_caps_WIDTH / keyboard_caps_TILE_W, keyboard_caps_HEIGHT / keyboard_caps_TILE_H,
                                             keyboard_caps_tiles, BANK(keyboard_caps)
                                            );
                } else {
                    screen_load_image_banked(menu->x + ((menu->width - (keyboard_WIDTH / keyboard_TILE_W)) >> 1), menu->y + 2,
                                             keyboard_WIDTH / keyboard_TILE_W, keyboard_HEIGHT / keyboard_TILE_H,
                                             keyboard_tiles, BANK(keyboard)
                                            );
                }
                break;
            default:
                if ((len = strlen(edit_text)) < edit_max_len) {
                    edit_text[len++] = letter;
                    edit_text[len] = 0;
                    menu_move_selection(menu, NULL, EditItems);
                }
                break;
        }
    }

    sync_vblank();

    return 0;
}
uint8_t onShowEdit(const menu_t * self, uint8_t * param) {
    edit_text = param;

    edit_lim_left = edit_pointer_x = ((self->x + 1) << 3);
    edit_lim_right = ((self->x + self->width - 1) << 3) - 1;
    edit_lim_top = edit_pointer_y = ((self->y + 2) << 3);
    edit_lim_bottom = ((self->y + self->height) << 3) - 2;

    menu_draw_frame(self);
    screen_load_image_banked(self->x + ((self->width - (keyboard_WIDTH / keyboard_TILE_W)) >> 1), self->y + 2, keyboard_WIDTH / keyboard_TILE_W, keyboard_HEIGHT / keyboard_TILE_H, keyboard_tiles, BANK(keyboard));
    screen_restore_rect(self->x + ((self->width - (keyboard_WIDTH / keyboard_TILE_W)) >> 1), self->y + 2, keyboard_WIDTH / keyboard_TILE_W, keyboard_HEIGHT / keyboard_TILE_H);
    return MENU_PROP_NO_FRAME;
}
uint8_t onTranslateKeyEdit(const struct menu_t * menu, const struct menu_item_t * self, uint8_t value) {
    menu; self;

    if (KEY_DOWN(J_LEFT)) {
        if (--edit_pointer_x < edit_lim_left) edit_pointer_x = edit_lim_left;
    } else if (KEY_DOWN(J_RIGHT)) {
        if (++edit_pointer_x > edit_lim_right) edit_pointer_x = edit_lim_right;
    }
    if (KEY_DOWN(J_UP)) {
        if (--edit_pointer_y < edit_lim_top) edit_pointer_y = edit_lim_top;
    } else if (KEY_DOWN(J_DOWN)) {
        if (++edit_pointer_y > edit_lim_bottom) edit_pointer_y = edit_lim_bottom;
    }

    return value & (J_A | J_B);
}

uint8_t ShowEdit(const uint8_t * msg, uint8_t max_len) BANKED {
    edit_shift_state = false;
    edit_max_len = max_len;
    uint8_t menu_result = menu_execute(&EditMenu, (uint8_t *)msg, NULL);
    hide_sprites_range(0, MAX_HARDWARE_SPRITES);
    JOYPAD_RESET();
    return menu_result;
}

uint8_t Edit(const uint8_t * msg, uint8_t max_len) NONBANKED {
    return ShowEdit(strcpy(text_buffer_extra, msg), max_len);
}