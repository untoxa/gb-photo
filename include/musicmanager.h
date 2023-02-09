#ifndef __MUSICMANAGER_H_INCLUDE__
#define __MUSICMANAGER_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#include "sfxplayer.h"
#if defined(NINTENDO)
#include "hUGEDriver.h"
#define MUSIC_MODULE hUGESong_t
#else
#define MUSIC_MODULE void
#endif

// SFX priority constants: concurrent effect will play only if its priority level is higher or equal
#define MUSIC_SFX_PRIORITY_MINIMAL  0
#define MUSIC_SFX_PRIORITY_NORMAL   4
#define MUSIC_SFX_PRIORITY_HIGH     8

// setting music bank to this value stops the playback
#define MUSIC_STOP_BANK 0xffu

// force cut channel after SFX is played (don't cut by default)
//#define FORCE_CUT_SFX

// masks applied to the ISR skip counter
#define MUSIC_TICK_MASK_60HZ 0x00u
#define MUSIC_TICK_MASK_256HZ 0x03u

extern volatile uint8_t music_current_track_bank;
extern uint8_t music_mute_mask;
extern const MUSIC_MODULE * music_next_track;
extern uint8_t music_sfx_priority;
extern uint8_t music_tick_mask;

// set up timer interrupt to 256Hz and set up driver for 256Hz
inline void music_setup_timer_ex(uint8_t is_fast) {
#if defined(NINTENDO)
    TMA_REG = (is_fast) ? 0x80u : 0xC0u;
    TAC_REG = 0x07u;
#else
    is_fast;
#endif
    music_tick_mask = MUSIC_TICK_MASK_256HZ;
}

// set up timer interrupt to 256Hz and set up driver for 256Hz
inline void music_setup_timer(void) {
#if defined(NINTENDO)
    music_setup_timer_ex((_cpu == CGB_TYPE) && (*(UBYTE *)0x0143 & 0x80));
#endif
}

// set up driver for 60Hz
inline void music_setup_vblank(void) {
    music_tick_mask = MUSIC_TICK_MASK_60HZ;
}

// initialize the music and SFX driver
inline void music_init(void) {
    music_current_track_bank = MUSIC_STOP_BANK;
    sfx_sound_init();
    sfx_reset_sample();
    sfx_sound_cut();
}

// cut sound on all channels
inline void music_sound_cut(void) {
    sfx_sound_cut();
}

// music channel constants
#define MUSIC_CH_1 SFX_CH_1
#define MUSIC_CH_2 SFX_CH_2
#define MUSIC_CH_3 SFX_CH_3
#define MUSIC_CH_4 SFX_CH_4

// cut sound on the selected channels
inline void music_sound_cut_mask(uint8_t mask) {
    sfx_sound_cut_mask(mask);
}

// music and SFX driver interrupt routine
void music_play_isr(void);

// load the music module
inline void music_load(uint8_t bank, const MUSIC_MODULE * data) {
    music_current_track_bank = MUSIC_STOP_BANK, music_next_track = data; music_current_track_bank = bank;
}

// pause music
void music_pause(uint8_t pause);

// stop music
inline void music_stop() {
    music_current_track_bank = MUSIC_STOP_BANK, music_sound_cut();
}

// muting masks
#define MUTE_MASK_NONE 0
#define MUTE_MASK_WAVE MUSIC_CH_3

// play SFX with the desired priority, muting the music on the selected channels
inline void music_play_sfx(uint8_t bank, const uint8_t * sample, uint8_t mute_mask, uint8_t priority) {
    if (priority < music_sfx_priority) return;
    sfx_play_bank = SFX_STOP_BANK;
    music_sfx_priority = priority;
    music_sound_cut_mask(music_mute_mask);
    music_mute_mask = mute_mask;
    sfx_set_sample(bank, sample);
}

#endif