#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <string.h>

#include "globals.h"
#include "states.h"
#include "load_save.h"

#include "systemdetect.h"
#include "systemhelpers.h"
#include "joy.h"
#include "musicmanager.h"
#include "bankdata.h"
#include "screen.h"
#include "vwf.h"
#include "remote.h"
#include "misc_assets.h"
#include "palette.h"
#include "fade_manager.h"

// graphic assets
#include "cursors.h"
#include "font_proportional.h"

// audio assets
#include "music.h"


#define _STATE(STATE_ID) DECLARE_STATE(STATE_ID)
STATES
#undef _STATE

#define _STATE(STATE_ID) { \
    .INIT = {.SEG = BANK(STATE_ID), .OFS = (void *)INIT_##STATE_ID}, \
    .ENTER = {.SEG = BANK(STATE_ID), .OFS = (void *)ENTER_##STATE_ID}, \
    .UPDATE = {.SEG = BANK(STATE_ID), .OFS = (void *)UPDATE_##STATE_ID}, \
    .LEAVE = {.SEG = BANK(STATE_ID), .OFS = (void *)LEAVE_##STATE_ID} \
},
const state_desc_t PROGRAM_STATES[] = {
    STATES
};
#undef _STATE

STATE OLD_PROGRAM_STATE = N_STATES, CURRENT_PROGRAM_STATE = DEFAULT_STATE;

#if defined(NINTENDO)
void hUGETrackerRoutine(unsigned char ch, unsigned char param, unsigned char tick) NONBANKED OLDCALL {
    ch; param; tick;
}
void LCD_ISR() {
    if (LYC_REG == 95) {
        while (STAT_REG & STATF_BUSY);
        LCDC_REG |= LCDCF_BG8000;
        LYC_REG = 150;
    } else {
        LCDC_REG &= ~LCDCF_BG8000;
        LYC_REG = 95;
    }
}
#endif

void main() {
    static uint8_t music_paused = FALSE;

    ENABLE_RAM;
    DISPLAY_OFF;

    detect_system();    // detect system compatibility

    palette_init();
    fade_init();
    if (_is_COLOR) {
        palette_cgb_zero(BCPS_REG_ADDR);
        palette_cgb_zero(OCPS_REG_ADDR);
    }
    fade_setspeed(1);

#if (USE_CGB_DOUBLE_SPEED==1)
    CPU_FAST();
#endif

    init_save_structure();

    music_init();
    CRITICAL {
#if defined(NINTENDO)
        LYC_REG = 144, STAT_REG |= STATF_LYC;
        add_LCD(LCD_ISR);
#endif
        music_setup_timer_ex(_is_CPU_FAST);
        add_low_priority_TIM(music_play_isr);
    }
#if defined(NINTENDO)
    set_interrupts(VBL_IFLAG | LCD_IFLAG | TIM_IFLAG);
#else
    set_interrupts(VBL_IFLAG | TIM_IFLAG);
#endif
    music_load(BANK(music_ingame), &music_ingame), music_pause(music_paused = TRUE);

    JOYPAD_INIT;

    remote_init();
    remote_activate(REMOTE_ENABLED);

    misc_assets_init(); // load some assets (menu corners, solid black/white blocks)

    // initialize the screen
    if (_is_COLOR) {
        VBK_REG = 1;
        fill_bkg_rect(0, 0, 20, 18, 0);
        VBK_REG = 0;
    }
    screen_clear_rect(0, 0, 20, 18, SOLID_BLACK);
    SHOW_BKG; SHOW_SPRITES; SPRITES_8x8;
    DISPLAY_ON;

//        if (joy & J_UP)     music_play_sfx(BANK(sound_effect1), sound_effect1, SFX_MUTE_MASK(sound_effect1));
//        if (joy & J_LEFT)   music_play_sfx(BANK(wave_icq_message), wave_icq_message, SFX_MUTE_MASK(wave_icq_message));
//        if (joy & J_SELECT) music_stop(), music_pause(music_paused = FALSE);
//        if (joy & J_START)  music_pause(music_paused = (!music_paused));

    vwf_load_font(0, font_proportional, BANK(font_proportional));
    vwf_activate_font(0);
//    vwf_set_colors(2, 1);

    // call init for each state
    for (uint8_t i = 0; i != N_STATES; i++) call_far(&PROGRAM_STATES[i].INIT);

    // main program loop
    while (TRUE) {
        if (OLD_PROGRAM_STATE != CURRENT_PROGRAM_STATE) {
            if (OLD_PROGRAM_STATE < N_STATES) call_far(&PROGRAM_STATES[OLD_PROGRAM_STATE].LEAVE);
            call_far(&PROGRAM_STATES[OLD_PROGRAM_STATE = CURRENT_PROGRAM_STATE].ENTER);
        }
        if (call_far(&PROGRAM_STATES[CURRENT_PROGRAM_STATE].UPDATE)) wait_vbl_done();
    }
}
