#include <gbdk/platform.h>
#include <stdint.h>

#include "musicmanager.h"
#include "sfxplayer.h"

#if defined(NINTENDO)
#include "hUGEDriver.h"
#endif

volatile uint8_t music_current_track_bank = MUSIC_STOP_BANK;
uint8_t music_mute_flag = FALSE, music_mute_mask = 0;
const MUSIC_MODULE * music_next_track;
uint8_t music_play_isr_counter = 0;
uint8_t music_play_isr_pause = FALSE;

void music_play_isr() NONBANKED {
    if (sfx_play_bank != SFX_STOP_BANK) {
        if (!music_mute_flag) {
#if defined(NINTENDO)
            hUGE_mute_mask = music_mute_mask; 
#endif
            music_mute_flag = TRUE;
        }
        if (!sfx_play_isr()) {
#if defined(NINTENDO)
            hUGE_mute_mask = 0, hUGE_reset_wave(); 
#endif
            music_mute_flag = FALSE;
            #ifdef FORCE_CUT_SFX
            music_sound_cut_mask(music_mute_mask);
            #endif
            music_mute_mask = 0; 
            sfx_play_bank = SFX_STOP_BANK;
        }
    }
    if (music_play_isr_pause) return;
    if (music_current_track_bank == MUSIC_STOP_BANK) return;
    if (++music_play_isr_counter & 3) return;
    uint8_t save_bank = _current_bank;
    SWITCH_ROM(music_current_track_bank);
    if (music_next_track) {
        music_sound_cut();
#if defined(NINTENDO)
        hUGE_init(music_next_track);
#endif
        music_next_track = 0;
    } else {
#if defined(NINTENDO)
        hUGE_dosound();
#endif
    }
    SWITCH_ROM(save_bank);
}

void music_pause(uint8_t pause) {
    if (music_play_isr_pause = pause) music_sound_cut();
}
