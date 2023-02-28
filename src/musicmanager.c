#include <gbdk/platform.h>
#include <stdint.h>

#include "musicmanager.h"
#include "sfxplayer.h"

#if defined(NINTENDO)
#include "hUGEDriver.h"
#endif

volatile uint8_t music_current_track_bank = MUSIC_STOP_BANK;
volatile uint8_t music_mute_mask = MUTE_MASK_NONE;
const MUSIC_MODULE * music_next_track;
uint8_t music_play_isr_counter = 0;
uint8_t music_play_isr_pause = FALSE;
volatile uint8_t music_sfx_priority = MUSIC_SFX_PRIORITY_MINIMAL;
uint8_t music_tick_mask = MUSIC_TICK_MASK_256HZ;

void music_play_isr(void) NONBANKED {
    if (sfx_play_bank != SFX_STOP_BANK) {
#if defined(NINTENDO)
        hUGE_mute_mask = music_mute_mask;
#endif
        if (!sfx_play_isr()) {
#if defined(NINTENDO)
            hUGE_mute_mask = MUTE_MASK_NONE, hUGE_reset_wave();
#endif
#ifdef FORCE_CUT_SFX
            music_sound_cut_mask(music_mute_mask);
#endif
            music_mute_mask = MUTE_MASK_NONE;
            music_sfx_priority = MUSIC_SFX_PRIORITY_MINIMAL;
            sfx_play_bank = SFX_STOP_BANK;
        }
    }
    if (music_play_isr_pause) return;
    if (music_current_track_bank == MUSIC_STOP_BANK) return;
    if (++music_play_isr_counter & music_tick_mask) return;
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
