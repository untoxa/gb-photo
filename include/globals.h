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
    #define DEBUG_ENABLED 1
#endif
// enable usage of CGB double speed mode
#ifndef USE_CGB_DOUBLE_SPEED
    #define USE_CGB_DOUBLE_SPEED 1
#endif
// enable support for Pic'N'Rec
#ifndef PICNREC_ENABLED
    #define PICNREC_ENABLED 1
#endif

extern uint8_t text_buffer[MAX_TEXT_BUFFER_SIZE];   // temporary buffer for rendering of text

#define STATES \
_STATE(state_logo)\
_STATE(state_gallery)\
_STATE(state_thumbnails)\
_STATE(state_camera)\
STATE_DEF_END

#define DEFAULT_STATE state_logo
#define MAIN_STATE state_camera

#endif