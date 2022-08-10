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
// enable usage of CGB double speed mode
#ifndef USE_CGB_DOUBLE_SPEED
    #define USE_CGB_DOUBLE_SPEED 0
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
// pid controller
#ifndef ENABLE_PID
    #define ENABLE_PID 1
    #ifndef PID_ENABLE_I
        #define PID_ENABLE_I 1
    #endif
    #ifndef PID_ENABLE_D
        #define PID_ENABLE_D 0
    #endif
    #define RENDER_ALL_REGS 0
    #define DEBUG_PID 0
#endif
// iterate mode
#ifndef ITERATE_ENABLED
    #define ITERATE_ENABLED 0
#endif
// use interrupts to update joypad state
#ifndef INT_DRIVEN_JOYPAD
    #define INT_DRIVEN_JOYPAD 1
#endif

extern uint8_t text_buffer[MAX_TEXT_BUFFER_SIZE];   // temporary buffer for rendering of text
#define text_buffer_extra (text_buffer + (MAX_TEXT_BUFFER_SIZE / 3))
#define text_buffer_extra_ex (text_buffer + ((MAX_TEXT_BUFFER_SIZE / 3) * 2))

#define STATES \
_STATE(state_logo)\
_STATE(state_gallery)\
_STATE(state_thumbnails)\
_STATE(state_camera)\
STATE_DEF_END

#define DEFAULT_STATE state_logo
#define MAIN_STATE state_camera

#endif