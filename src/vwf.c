#include "vwf.h"

#if defined(NINTENDO)
#define VWF_DEFAULT_BASE_ADDRESS 0x9800
#define VWF_TILE_SIZE_BITS 4
#elif defined(SEGA)
#define VWF_DEFAULT_BASE_ADDRESS 0x7800
#define VWF_TILE_SIZE_BITS 5
#endif
#define DEVICE_TILE_SIZE 8u
#define VWF_TILE_SIZE (1 << VWF_TILE_SIZE_BITS)

vwf_farptr_t vwf_fonts[4];

uint8_t vwf_current_offset = 0;
uint8_t vwf_tile_data[DEVICE_TILE_SIZE * 2];
uint8_t vwf_current_mask;
uint8_t vwf_current_rotate;
uint8_t vwf_inverse_map = 0;
uint8_t * vwf_current_tile;
uint8_t vwf_tab_size = 2;

font_desc_t vwf_current_font_desc;
uint8_t vwf_current_font_bank;

#if defined(NINTENDO)
void vwf_print_shift_char(void * dest, const void * src, uint8_t bank) OLDCALL;
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank) OLDCALL;
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank) OLDCALL __preserves_regs(b, c) ;
uint8_t * vwf_get_win_addr() OLDCALL __preserves_regs(b, c, h, l) ;
uint8_t * vwf_get_bkg_addr() OLDCALL __preserves_regs(b, c, h, l) ;
void vwf_set_banked_data(uint8_t i, uint8_t l, const unsigned char* ptr, uint8_t bank) OLDCALL;
void vwf_swap_tiles() OLDCALL;
#elif defined(SEGA)
void vwf_print_shift_char(void * dest, const void * src, uint8_t bank) __z88dk_callee;
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank) __z88dk_callee;
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank) __z88dk_callee;
uint8_t * vwf_get_win_addr() OLDCALL;
uint8_t * vwf_get_bkg_addr() OLDCALL;
void vwf_set_banked_data(uint8_t i, uint8_t l, const unsigned char* ptr, uint8_t bank) __z88dk_callee;
void vwf_swap_tiles() OLDCALL;
#endif

void set_1bpp_data(uint8_t *first_tile, uint8_t nb_tiles, const uint8_t *data) OLDCALL PRESERVES_REGS(b, c);

void vwf_print_reset(uint8_t * tile) {
    vwf_current_tile = tile;
    vwf_current_offset = 0;
    vwf_swap_tiles(); 
    vwf_swap_tiles(); 
}

uint8_t vwf_print_render(const unsigned char ch) {
    uint8_t letter = vwf_read_banked_ubyte(vwf_current_font_desc.recode_table + (ch & ((vwf_current_font_desc.attr & RECODE_7BIT) ? 0x7fu : 0xffu)), vwf_current_font_bank);
    const uint8_t * bitmap = vwf_current_font_desc.bitmaps + (uint16_t)letter * 8;
    if (vwf_current_font_desc.attr & FONT_VWF) {
        uint8_t width = vwf_read_banked_ubyte(vwf_current_font_desc.widths + letter, vwf_current_font_bank);
        uint8_t dx = (8u - vwf_current_offset);
        vwf_current_mask = (0xffu << dx) | (0xffu >> (vwf_current_offset + width));

        vwf_current_rotate = vwf_current_offset;
        vwf_print_shift_char(vwf_tile_data, bitmap, vwf_current_font_bank);
        if ((uint8_t)(vwf_current_offset + width) > 8u) {
            vwf_current_rotate = dx | 0x80u;
            vwf_current_mask = 0xffu >> (width - dx);
            vwf_print_shift_char(vwf_tile_data + DEVICE_TILE_SIZE, bitmap, vwf_current_font_bank);
        }
        vwf_current_offset += width;

        if (vwf_current_offset > 7u) {
            vwf_current_offset -= 8u;
            set_1bpp_data(vwf_current_tile, (vwf_current_offset) ? 2 : 1, vwf_tile_data);
            vwf_current_tile += VWF_TILE_SIZE;
            vwf_swap_tiles();
            return TRUE;
        };
        set_1bpp_data(vwf_current_tile, 1, vwf_tile_data);
        return FALSE;
    } else {
//        vwf_set_banked_data(vwf_current_tile += VWF_TILE_SIZE, 1, bitmap, vwf_current_font_bank);
        vwf_current_offset = 0;
        return TRUE;
    }
}

uint8_t vwf_draw_text(uint8_t * base_tile, const unsigned char * str) {
    static const uint8_t * ui_text_ptr;
    ui_text_ptr = str;

    vwf_print_reset(base_tile);
    while (*ui_text_ptr) {
        switch (*ui_text_ptr) {
            case 0x01:
                vwf_activate_font(*++ui_text_ptr);
                break;
            case 0x03:
                vwf_inverse_map = *++ui_text_ptr;
                break;
            case '\t':
                vwf_print_reset(vwf_next_tile());
                while ((uint8_t)((uint16_t)(vwf_current_tile - base_tile) >> VWF_TILE_SIZE_BITS) % vwf_tab_size) {
                    set_1bpp_data(vwf_current_tile, 1, vwf_tile_data);
                    vwf_current_tile += VWF_TILE_SIZE;
                    vwf_swap_tiles();
                }
                break;
            default:
                vwf_print_render(*ui_text_ptr);
                break;
        }
        ui_text_ptr++;
    }

    return  ((uint16_t)(vwf_current_tile - base_tile) >> VWF_TILE_SIZE_BITS) + ((vwf_current_offset) ? 1 : 0);
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

uint8_t * vwf_next_tile() {
    return (vwf_current_offset) ? vwf_current_tile + VWF_TILE_SIZE : vwf_current_tile;
}