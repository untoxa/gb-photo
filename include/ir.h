#ifndef __IR_H_INCLUDE__
#define __IR_H_INCLUDE__

// Turn on IR sensing only on CGB
void ir_sense_start();

// Turn off IR sensing only on CGB
void ir_sense_stop();

// If IR is detected, sense for a short period and return true 
// if a pattern of flashes follows
// Returns false immediately if not CGB, or no IR is sensed time of call
uint8_t ir_sense_pattern();

#endif