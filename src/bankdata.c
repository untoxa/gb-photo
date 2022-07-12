#include <gbdk/platform.h>
#include <stdint.h>
#include <string.h>

static uint8_t _save;

void * banked_memcpy(void *dest, const void *src, size_t len, uint8_t bank) NONBANKED {
    _save = _current_bank;
    SWITCH_ROM(bank);
    void * res = memcpy(dest, src, len);
    SWITCH_ROM(_save);
    return res;
}

void set_banked_sprite_data(uint8_t first_tile, uint8_t nb_tiles, const uint8_t *data, uint8_t bank) NONBANKED {
    _save = _current_bank;
    SWITCH_ROM(bank);
    set_sprite_data(first_tile, nb_tiles, data);
    SWITCH_ROM(_save);
}

void set_banked_bkg_data(uint8_t first_tile, uint8_t nb_tiles, const uint8_t *data, uint8_t bank) NONBANKED {
    _save = _current_bank;
    SWITCH_ROM(bank);
    set_bkg_data(first_tile, nb_tiles, data);
    SWITCH_ROM(_save);
}

void set_banked_bkg_submap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *map, uint8_t map_w, uint8_t bank) NONBANKED {
    _save = _current_bank;
    SWITCH_ROM(bank);
    set_bkg_submap(x, y, w, h, map, map_w);
    SWITCH_ROM(_save);
}

uint8_t read_banked_ubyte(const uint8_t * ptr, uint8_t bank) NONBANKED {
    _save = _current_bank;
    SWITCH_ROM(bank);
    uint8_t res = *ptr;
    SWITCH_ROM(_save);
    return res;
}