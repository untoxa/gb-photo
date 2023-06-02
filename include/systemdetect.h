#ifndef __SYSTEMDETECT_H_INCLUDE__
#define __SYSTEMDETECT_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>

#include "globals.h"

BANKREF_EXTERN(module_detect_system)

extern bool _is_SUPER, _is_COLOR, _is_ADVANCE;
extern bool _is_CPU_FAST;

uint8_t CPU_FAST(void) BANKED;
void CPU_SLOW(void) BANKED;

uint8_t INIT_module_detect_system(void) BANKED;

#endif