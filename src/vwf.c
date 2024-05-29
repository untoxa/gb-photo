#include "vwf.h"

#define DEVICE_TILE_WIDTH 8u
#define DEVICE_TILE_SIZE (1 << DEVICE_TILE_SIZE_BITS)
#define VWF_TILE_SIZE 8u

vwf_farptr_t vwf_fonts[4];

uint8_t vwf_current_offset = 0;
uint8_t vwf_tile_data[VWF_TILE_SIZE * 2];
uint8_t vwf_current_rotate;
uint8_t * vwf_current_tile;
uint8_t vwf_tab_size = 2;

font_desc_t vwf_current_font_desc;
uint8_t vwf_current_font_bank;

#if defined(NINTENDO)
void vwf_print_shift_char_right(void * dest, const void * src, uint8_t bank);
void vwf_print_shift_char_left(void * dest, const void * src, uint8_t bank);
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank);
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank);
void vwf_swap_tiles(void);
void set_1bpp_data(uint8_t *first_tile, uint8_t nb_tiles, const uint8_t *data);
#elif defined(SEGA)
void vwf_print_shift_char_right(void * dest, const void * src, uint8_t bank) __sdcccall(0) __z88dk_callee;
void vwf_print_shift_char_left(void * dest, const void * src, uint8_t bank) __sdcccall(0) __z88dk_callee;
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank) __sdcccall(0) __z88dk_callee;
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank) __sdcccall(0) __z88dk_callee;
void vwf_swap_tiles(void);
void set_1bpp_data(uint8_t *first_tile, uint16_t nb_tiles, const uint8_t *data) __sdcccall(0) __z88dk_callee;
#endif

void vwf_print_reset(const uint8_t * tile, uint8_t offset) {
    vwf_current_tile = (uint8_t *)tile;
    vwf_current_offset = offset & 0x07;
    vwf_swap_tiles();
    vwf_swap_tiles();
}

uint8_t vwf_print_render(const unsigned char ch) {
    uint8_t letter = vwf_read_banked_ubyte(vwf_current_font_desc.recode_table + (ch & ((vwf_current_font_desc.attr & RECODE_7BIT) ? 0x7fu : 0xffu)), vwf_current_font_bank);
    const uint8_t * bitmap = vwf_current_font_desc.bitmaps + (uint16_t)letter * 8;
    if (vwf_current_font_desc.attr & FONT_VWF) {
        uint8_t width = vwf_read_banked_ubyte(vwf_current_font_desc.widths + letter, vwf_current_font_bank);

        vwf_current_rotate = vwf_current_offset;
        vwf_print_shift_char_right(vwf_tile_data, bitmap, vwf_current_font_bank);
        if ((uint8_t)(vwf_current_offset + width) > 8u) {
            vwf_current_rotate = (8u - vwf_current_offset);
            vwf_print_shift_char_left(vwf_tile_data + VWF_TILE_SIZE, bitmap, vwf_current_font_bank);
        }
        vwf_current_offset += width;

        if (vwf_current_offset > 7u) {
            vwf_current_offset -= 8u;
            set_1bpp_data(vwf_current_tile, 1, vwf_tile_data);
            vwf_current_tile += DEVICE_TILE_SIZE;
            vwf_swap_tiles();
            return TRUE;
        };
        return FALSE;
    } else {
        // not implemented
        vwf_current_offset = 0;
        return TRUE;
    }
}

uint8_t vwf_text_width(const unsigned char * str) {
    uint8_t total_width = 0;
    if (vwf_current_font_desc.attr & FONT_VWF) {
        while(*str) {
            uint8_t letter;
            switch (*str) {
                case 0x01:
                case 0x03:
                    str++;
                    break;
                case 0x02:
                    str++;
                default:
                    letter = vwf_read_banked_ubyte(vwf_current_font_desc.recode_table + (*str & ((vwf_current_font_desc.attr & RECODE_7BIT) ? 0x7fu : 0xffu)), vwf_current_font_bank);
                    total_width += vwf_read_banked_ubyte(vwf_current_font_desc.widths + letter, vwf_current_font_bank);
                    break;
            }
            str++;
        }
    } else {
        while(*str++) total_width += DEVICE_TILE_WIDTH;
    }
    return total_width;
}

uint8_t vwf_draw_text(const uint8_t * base_tile, const unsigned char * str, uint8_t offset) {
    static const uint8_t * ui_text_ptr;
    ui_text_ptr = str;

    vwf_print_reset(base_tile, offset);
    while (*ui_text_ptr) {
        switch (*ui_text_ptr) {
            case 0x01:
                // activate font
                vwf_activate_font(*++ui_text_ptr);
                break;
            case 0x02:
                // escape character
                vwf_print_render(*++ui_text_ptr);
                break;
            case '\t':
                // tab character
                set_1bpp_data(vwf_current_tile, 1, vwf_tile_data);
                if (!vwf_current_offset) {
                    vwf_current_tile += DEVICE_TILE_SIZE;
                    vwf_swap_tiles();
                } else vwf_print_reset(vwf_next_tile(), 0);
                while ((uint8_t)((uint16_t)(vwf_current_tile - base_tile) >> DEVICE_TILE_SIZE_BITS) % vwf_tab_size) {
                    set_1bpp_data(vwf_current_tile, 1, vwf_tile_data);
                    vwf_current_tile += DEVICE_TILE_SIZE;
                    vwf_swap_tiles();
                }
                break;
            default:
                vwf_print_render(*ui_text_ptr);
                break;
        }
        ui_text_ptr++;
    }
    if (vwf_current_offset) set_1bpp_data(vwf_current_tile, 1, vwf_tile_data);

    return  ((uint16_t)(vwf_current_tile - base_tile) >> DEVICE_TILE_SIZE_BITS) + ((vwf_current_offset) ? 1 : 0);
}

void vwf_load_font(uint8_t idx, const void * font, uint8_t bank) {
    vwf_fonts[idx].bank = bank;
    vwf_fonts[idx].ptr = (void *)font;
    vwf_activate_font(idx);
}

void vwf_activate_font(uint8_t idx) {
    vwf_current_font_bank = vwf_fonts[idx].bank;
    vwf_memcpy(&vwf_current_font_desc, vwf_fonts[idx].ptr, sizeof(font_desc_t), vwf_current_font_bank);
}

uint8_t * vwf_next_tile(void) {
    return (vwf_current_offset) ? vwf_current_tile + DEVICE_TILE_SIZE : vwf_current_tile;
}