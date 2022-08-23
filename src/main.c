#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>
#include <string.h>

#include "globals.h"
#include "states.h"
#include "modules.h"

#include "systemdetect.h"
#include "bankdata.h"

// program MODULES
#define _MODULE(MODULE_ID) DECLARE_MODULE(MODULE_ID)
MODULES
#undef _MODULE

#define _MODULE(MODULE_ID) { \
    .INIT = { .SEG = BANK(MODULE_ID), .OFS = (void *)INIT_##MODULE_ID }, \
},
const module_desc_t PROGRAM_MODULES[] = {
    MODULES
};
#undef _MODULE

// program STATES
#define _STATE(STATE_ID) DECLARE_STATE(STATE_ID)
STATES
#undef _STATE   

#define _STATE(STATE_ID) { \
    .INIT   = { .SEG = BANK(STATE_ID), .OFS = (void *)INIT_##STATE_ID   }, \
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
    // call init for the each module
    for (uint8_t i = 0; i != N_MODULES; i++) call_far(&PROGRAM_MODULES[i].INIT);

    // call init for the each state
    for (uint8_t i = 0; i != N_STATES; i++) call_far(&PROGRAM_STATES[i].INIT);

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
        if (call_far(&PROGRAM_STATES[CURRENT_PROGRAM_STATE].UPDATE)) wait_vbl_done();
    }
}
