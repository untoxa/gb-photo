#ifndef __IR_H_INCLUDE__
#define __IR_H_INCLUDE__

// Turn on IR sensing only on CGB
inline void ir_sense_start(void) {
    if (_cpu == CGB_TYPE) {
        // Turn off IR emit & turn on IR read
        RP_REG = RPF_ENREAD;
    }
}


// Turn off IR sensing only on CGB
inline void ir_sense_stop(void) {
    if (_cpu == CGB_TYPE) {
        // Turn off IR emit & read
        RP_REG = 0;
    }
}

// If IR is detected, sense for a short period and return true 
// if a pattern of flashes follows
// Returns false immediately if not CGB, or no IR is sensed time of call
uint8_t ir_sense_pattern(void);

#endif