#pragma bank 255

#include <gbdk/platform.h>

#include <stdint.h>
#include <string.h>

#include "flasher.h"
#include "gbcamera.h"

#define FIRST_HALF_OFS 0
#define FIRST_HALF_LEN 8
#define SECOND_HALF_OFS 8
#define SECOND_HALF_LEN 8

extern uint8_t save_rom_bank;
extern uint8_t save_sram_bank_offset;

static cam_game_data_t AT(0x51B2) saved_game_data;

inline uint8_t slot_to_bank(uint8_t slot, uint8_t ofs) {
    return (((slot + 2) << 1) + ofs) << 2;
}

uint8_t flash_check_gallery_exist(uint8_t slot) NONBANKED {
    uint8_t _save = _current_bank, result;
    SWITCH_ROM(slot_to_bank(slot, 0));
    result = ((saved_game_data.magic[0] == 'M') &&
              (saved_game_data.magic[1] == 'a') &&
              (saved_game_data.magic[2] == 'g') &&
              (saved_game_data.magic[3] == 'i') &&
              (saved_game_data.magic[4] == 'c'));
    SWITCH_ROM(_save);
    return result;
}

void restore_sram_bank(uint8_t bank) NONBANKED {
    uint8_t _save = _current_bank;
    SWITCH_ROM(save_rom_bank + (bank >> 1));
    memcpy((uint8_t *)0xA000, (uint8_t *)(0x4000 + ((bank & 1) << 13)), 0x2000);
    SWITCH_ROM(_save);
}

uint8_t flash_load_gallery_from_slot(uint8_t slot) BANKED {
    // check saved gallery exist
    if (!flash_check_gallery_exist(slot)) return FALSE;
    // read 8 SRAM banks from the flash sector
    save_rom_bank = slot_to_bank(slot, 0);
    save_sram_bank_offset = FIRST_HALF_OFS;
    for (uint8_t i = 0; i < FIRST_HALF_LEN; i++) {
        SWITCH_RAM(i + FIRST_HALF_OFS);
        restore_sram_bank(i);
    }
    // read the next 8 SRAM banks from the next flash sector
    save_rom_bank = slot_to_bank(slot, 1);
    save_sram_bank_offset = SECOND_HALF_OFS;
    for (uint8_t i = 0; i < SECOND_HALF_LEN; i++) {
        SWITCH_RAM(i + SECOND_HALF_OFS);
        restore_sram_bank(i);
    }
    return TRUE;
}

extern uint8_t erase_flash() OLDCALL;                   // erases FLASH sector: 64K or 4 banks
extern uint8_t save_sram_banks(uint8_t count) OLDCALL;  // copies up to count SRAM banks to FLASH

uint8_t flash_save_gallery_to_slot(uint8_t slot) BANKED {
    // erase the sector and save first 8 SRAM banks
    save_sram_bank_offset = FIRST_HALF_OFS;
    save_rom_bank = slot_to_bank(slot, 0);
    if (!erase_flash()) return FALSE;
    if (!save_sram_banks(FIRST_HALF_LEN)) return FALSE;
    // erase the next sector and save the next 8 sram banks 
    save_sram_bank_offset = SECOND_HALF_OFS;
    save_rom_bank = slot_to_bank(slot, 1);
    if (!erase_flash()) return FALSE;
    return save_sram_banks(SECOND_HALF_LEN);            // update offset
}

uint8_t flash_erase_slot(uint8_t slot) BANKED {
    save_rom_bank = slot_to_bank(slot, 0);
    if (!erase_flash()) return FALSE;
    save_rom_bank = slot_to_bank(slot, 1);
    return erase_flash();
}
