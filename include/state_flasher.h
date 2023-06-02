#ifndef __STATE_FLASHER_H_INCLUDE__
#define __STATE_FLASHER_H_INCLUDE__

#include <stdint.h>

#include "menus.h"

#include "misc_assets.h"

#define MAX_FLASHER_THUMBNAILS 10
#define MAX_FLASHER_THUMBS_PAGES 3
#define FLASHER_THUMBS_COUNT_X 5
#define FLASHER_THUMBS_COUNT_Y 2

#define FLASHER_THUMBS_DISPLAY_X 0
#define FLASHER_THUMBS_DISPLAY_Y 9
#define FLASHER_THUMBS_DISPLAY_WIDTH (FLASHER_THUMBS_COUNT_X * 4)
#define FLASHER_THUMBS_DISPLAY_HEIGHT (FLASHER_THUMBS_COUNT_Y * 4)

#define FLASHER_FOLDER_DISPLAY_X1 2
#define FLASHER_FOLDER_DISPLAY_Y1 1
#define FLASHER_FOLDER_DISPLAY_ROW1 (4 * 4)
#define FLASHER_FOLDER_DISPLAY_X2 4
#define FLASHER_FOLDER_DISPLAY_Y2 5
#define FLASHER_FOLDER_DISPLAY_ROW2 (3 * 4)

#define MAX_FLASH_SLOTS 7

#define FIRST_HALF_OFS 0
#define FIRST_HALF_LEN 8
#define SECOND_HALF_OFS 8
#define SECOND_HALF_LEN 8

BANKREF_EXTERN(state_flasher)

typedef enum {
    browse_mode_folders,
    browse_mode_thumbnails,
    N_BROWSE_MODES
} browse_mode_e;

typedef struct item_coord_t {
    uint8_t x, y;
} item_coord_t;

extern uint8_t save_rom_bank;
extern uint8_t save_sram_bank_offset;

inline uint8_t slot_to_sector(uint8_t slot, uint8_t ofs) {
    return ((((slot) + 1) << 3) + (ofs << 2));
}

inline void flasher_show_progressbar(uint8_t x, uint8_t value, uint8_t size) {
    misc_render_progressbar(value, size, text_buffer);
    menu_text_out(x, 17, HELP_CONTEXT_WIDTH, WHITE_ON_BLACK, text_buffer);
}

extern uint8_t erase_flash(void) OLDCALL BANKED;                   // erases FLASH sector: 64K or 4 banks
extern uint8_t save_sram_banks(uint8_t count) OLDCALL BANKED;  // copies up to count SRAM banks to FLASH

#endif