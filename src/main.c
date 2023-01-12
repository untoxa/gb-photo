#include <gbdk/platform.h>
#include <stdint.h>

#include "systemhelpers.h"
#include "bankdata.h"

#include "globals.h"
#include "modules.h"
#include "states.h"

// declare program MODULES
#define _MODULE(MODULE_ID) DECLARE_MODULE(MODULE_ID)
MODULES
#undef _MODULE

// declare program STATES
#define _STATE(STATE_ID) DECLARE_STATE(STATE_ID)
STATES
#undef _STATE

// define array of initializers for the each MODULE and STATE (MODULES first)
#define _MODULE(MODULE_ID) { .SEG = BANK(MODULE_ID), .OFS = (void *)INIT_##MODULE_ID },
#define _STATE(STATE_ID)   { .SEG = BANK(STATE_ID),  .OFS = (void *)INIT_##STATE_ID  },
const far_ptr_t PROGRAM_MODULES[] = {
    MODULES
    STATES
};
#undef _MODULE
#undef _STATE

//define array of STATES callbacks
#define _STATE(STATE_ID) { \
    .ENTER  = { .SEG = BANK(STATE_ID), .OFS = (void *)ENTER_##STATE_ID  }, \
    .UPDATE = { .SEG = BANK(STATE_ID), .OFS = (void *)UPDATE_##STATE_ID }, \
    .LEAVE  = { .SEG = BANK(STATE_ID), .OFS = (void *)LEAVE_##STATE_ID  } \
},
const state_desc_t PROGRAM_STATES[] = {
    STATES
};
#undef _STATE

STATE OLD_PROGRAM_STATE = N_STATES, CURRENT_PROGRAM_STATE = DEFAULT_STATE;


void main() {
    // call init for the each module and then for the each state
    for (uint8_t i = 0; i != LENGTH(PROGRAM_MODULES); i++) call_far(&PROGRAM_MODULES[i]);

    // the main program loop
    while (TRUE) {
        // check if state had changed
        if (OLD_PROGRAM_STATE != CURRENT_PROGRAM_STATE) {
            // leave the old state if valid
            if (OLD_PROGRAM_STATE < N_STATES) call_far(&PROGRAM_STATES[OLD_PROGRAM_STATE].LEAVE);
            // enter the new state
            call_far(&PROGRAM_STATES[OLD_PROGRAM_STATE = CURRENT_PROGRAM_STATE].ENTER);
        }
        // call state update and wait for VBlank if requested
        if (call_far(&PROGRAM_STATES[CURRENT_PROGRAM_STATE].UPDATE)) vsync();
    }
}
