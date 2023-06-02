#pragma bank 255

#include "remote.h"

BANKREF(module_remote)

volatile uint8_t remote_keys = 0;

uint8_t INIT_module_remote(void) BANKED {
    return 0;
}