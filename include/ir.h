#ifndef __IR_H_INCLUDE__
#define __IR_H_INCLUDE__

#include <gbdk/platform.h>

#include "systemdetect.h"

// Turn on IR sensing only on CGB
inline void ir_sense_start(void) {
#if defined(NINTENDO)
    if (_is_COLOR) {
        // Turn off IR emit & turn on IR read
        RP_REG = RPF_ENREAD;
    }
#endif
}

// Turn off IR sensing only on CGB
inline void ir_sense_stop(void) {
#if defined(NINTENDO)
    if (_is_COLOR) {
        // Turn off IR emit & read
        RP_REG = 0;
    }
#endif
}

// If IR is detected, sense for a short period and return true 
// if a pattern of flashes follows
// Returns false immediately if not CGB, or no IR is sensed time of call
uint8_t ir_sense_pattern(void);

#endif