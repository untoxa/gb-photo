#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "systemdetect.h"
#include "musicmanager.h"
#include "module_music.h"

// audio assets
#include "music.h"

BANKREF(module_music)

uint8_t music_paused = FALSE;


//    // music is not used by now, examples for start/stop/pause are commented out and left for the reference
//    if (joy & J_SELECT) music_stop(), music_pause(music_paused = FALSE);
//    if (joy & J_START)  music_pause(music_paused = (!music_paused));


#if defined(NINTENDO)
void hUGETrackerRoutine(unsigned char ch, unsigned char param, unsigned char tick) NONBANKED OLDCALL {
    ch; param; tick;
}
#endif

// initialize the music/SFX driver
uint8_t INIT_module_music() BANKED {
    music_init();
    CRITICAL {
        music_setup_timer_ex(_is_CPU_FAST);
#if defined(NINTENDO)
        add_low_priority_TIM(music_play_isr);
#else
        add_VBL(music_play_isr);
#endif
    }
#if defined(NINTENDO)
    set_interrupts(IE_REG | TIM_IFLAG);
#endif
    music_load(BANK(music_ingame), &music_ingame), music_pause(music_paused = TRUE);
    return 0;
}