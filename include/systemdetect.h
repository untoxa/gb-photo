#ifndef __SYSTEMDETECT_H_INCLUDE__
#define __SYSTEMDETECT_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

extern uint8_t _is_SUPER, _is_COLOR, _is_ADVANCE;

void detect_system();

#endif