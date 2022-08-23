#ifndef __MODULES_H_INCLUDE__
#define __MODULES_H_INCLUDE__

#include <gbdk/platform.h>
#include "globals.h"
#include "states.h"

#define DECLARE_MODULE(MODULE_IDX) \
    uint8_t INIT_##MODULE_IDX() BANKED; \
    BANKREF_EXTERN(MODULE_IDX)

#define MODULE_DEF_END

#define _MODULE(MODULE_ID) MODULE_ID,
typedef enum {
	MODULES

	N_MODULES
} MODULE;
#undef _MODULE

#endif