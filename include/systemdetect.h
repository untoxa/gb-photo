#ifndef __SYSTEMDETECT_H_INCLUDE__
#define __SYSTEMDETECT_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

#include "globals.h"

extern uint8_t _is_SUPER, _is_COLOR, _is_ADVANCE;
extern uint8_t _is_CPU_FAST;

void detect_system();
void CPU_FAST(uint8_t display);
void CPU_SLOW(uint8_t display);

#endif