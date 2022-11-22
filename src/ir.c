#include <time.h>
#include <gbdk/platform.h>
#include <gb/hardware.h>

#include "ir.h"

void ir_sense_start() {
#ifdef _CGB_H
    // Turn off IR emit & turn on IR read
    RP_REG = RPF_ENREAD;
#endif
}

void ir_sense_stop() {
#ifdef _CGB_H
    // Turn off IR emit & read
    RP_REG = 0;
#endif
}

uint8_t ir_sense_pattern() {
#ifdef _CGB_H
    // If you sense IR, sample for a short period (1 is off)
    if ((RP_REG & RPF_DATAIN) == RPF_DATAIN) {
        return FALSE;
    }

    // How long to sense for
    const clock_t SENSE_DUR = CLOCKS_PER_SEC / 4;
    // How many full on/off cycles to ensure this is periodic flashing
    const uint8_t IR_FLASHES_MIN = 2;

    clock_t start_t = clock();
    uint8_t lastir = 1;
    uint8_t count = 1;
    // Changing CPU speed causes tearing on the next screen redraw - there's probably a good solve for this but it's beyond me
    //cpu_fast();
    do {
        uint8_t ir = (RP_REG & RPF_DATAIN) >> 1;
        count += (lastir ^ ir) & 0x1;
        lastir = ir;
    } while (clock() < start_t + SENSE_DUR);
    //cpu_slow();

    return (count >= 2 * IR_FLASHES_MIN);
#else
    return FALSE;
#endif
}
