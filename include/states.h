#ifndef __STATES_H_INCLUDE__
#define __STATES_H_INCLUDE__

#include <gbdk/platform.h>
#include "globals.h"

#define DECLARE_STATE(STATE_IDX) \
    uint8_t INIT_##STATE_IDX() BANKED; \
    uint8_t ENTER_##STATE_IDX() BANKED; \
    uint8_t UPDATE_##STATE_IDX() BANKED; \
    uint8_t LEAVE_##STATE_IDX() BANKED; \
    BANKREF_EXTERN(STATE_IDX)

#define STATE_DEF_END

#define _STATE(STATE_ID) STATE_ID,
typedef enum {
	STATES

	N_STATES
} STATE;
#undef _STATE

typedef struct far_ptr_t {
    uint8_t SEG;
    void * OFS;
} far_ptr_t;

typedef struct state_desc_t {
    far_ptr_t INIT;
    far_ptr_t ENTER;
    far_ptr_t UPDATE;
    far_ptr_t LEAVE;
} state_desc_t;

extern STATE CURRENT_PROGRAM_STATE, OLD_PROGRAM_STATE;

inline void CHANGE_STATE(STATE new_state) {
    CURRENT_PROGRAM_STATE = new_state;
}

inline uint8_t STATE_CHANGED() {
    return (CURRENT_PROGRAM_STATE != OLD_PROGRAM_STATE);
}

#endif