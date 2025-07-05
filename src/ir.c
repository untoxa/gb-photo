#include <time.h>
#include <gbdk/platform.h>

#include "systemdetect.h"
#include "ir.h"

// How long to sense for
#define SENSE_DUR CLOCKS_PER_SEC / 4
// How many full on/off cycles to ensure this is periodic flashing
#define IR_FLASHES_MIN 2


uint8_t ir_sense_pattern(void) {
#if defined(NINTENDO)
    if (!_is_COLOR) return FALSE;

    // If you sense IR, sample for a short period (1 is off)
    if ((RP_REG & RPF_DATAIN) == RPF_DATAIN) {
        return FALSE;
    }

    clock_t end_t = clock() + SENSE_DUR;
    uint8_t lastir = 1;
    uint8_t count = 1;
    do {
        uint8_t ir = (RP_REG & RPF_DATAIN) >> 1;
        count += (lastir ^ ir) & 0x1;
        lastir = ir;
    } while (clock() < end_t);

    return (count >= 2 * IR_FLASHES_MIN);
#else
    return FALSE;
#endif
}
