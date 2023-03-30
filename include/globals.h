#ifndef __GLOBALS_H_INCLUDE__
#define __GLOBALS_H_INCLUDE__

#include <stdint.h>

#define MAX_TEXT_BUFFER_SIZE 256

// coordinates of the main image display
#define IMAGE_DISPLAY_X 2
#define IMAGE_DISPLAY_Y 2

// width of the help context display in tiles
#define HELP_CONTEXT_WIDTH 17
// width of the slot usage display in tiles
#define IMAGE_SLOTS_USED_WIDTH (20 - HELP_CONTEXT_WIDTH)

// enable debug menu item
#ifndef DEBUG_ENABLED
    #define DEBUG_ENABLED 0
#endif
// enable fast printer transfers
#ifndef CGB_FAST_TRANSFER
    #define CGB_FAST_TRANSFER 1
#endif
// enable support for Pic'N'Rec
#ifndef PICNREC_ENABLED
    #define PICNREC_ENABLED 1
#endif
// shutter sound index
#ifndef SHUTTER_SOUND
    #define SHUTTER_SOUND 0
#endif
// auto-exposure
#define ENABLE_AUTOEXP
#ifdef ENABLE_AUTOEXP
    #define DEBUG_AUTOEXP 0
    #define AUTOEXP_LOW_LIMIT TO_EXPOSURE_VALUE(768)
    #define AUTOEXP_HIGH_LIMIT CAM02_MAX_VALUE
#endif
// bracketing mode
#ifndef BRACKETING_ENABLED
    #define BRACKETING_ENABLED 0
#endif
// use interrupts to update joypad state
#ifndef INT_DRIVEN_JOYPAD
    #define INT_DRIVEN_JOYPAD 1
#endif

extern uint8_t text_buffer[MAX_TEXT_BUFFER_SIZE];   // temporary buffer for rendering of text
#define text_buffer_extra (text_buffer + (MAX_TEXT_BUFFER_SIZE / 3))
#define text_buffer_extra_ex (text_buffer + ((MAX_TEXT_BUFFER_SIZE / 3) * 2))

// define modules: initialization order is important
#define MODULES \
_MODULE(module_detect_system)\
_MODULE(module_load_save)\
_MODULE(module_protected)\
_MODULE(module_sgb_assets)\
_MODULE(module_joy)\
_MODULE(module_splash)\
_MODULE(module_display_off)\
_MODULE(module_palette)\
_MODULE(module_music)\
_MODULE(module_screen)\
_MODULE(module_remote)\
_MODULE(module_misc_assets)\
_MODULE(module_vwf)\
_MODULE(module_display_on)\
MODULE_DEF_END

// define program states
#define STATES \
_STATE(state_logo)\
_STATE(state_gallery)\
_STATE(state_thumbnails)\
_STATE(state_camera)\
_STATE(state_flasher)\
STATE_DEF_END

#define DEFAULT_STATE state_logo
#define MAIN_STATE state_camera

#endif
