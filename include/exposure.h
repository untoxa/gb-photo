#ifndef __EXPOSURE_H_INCLUDE__
#define __EXPOSURE_H_INCLUDE__

#include <string.h>

extern uint16_t EXPOSURES_MAX_INDEX;
extern uint16_t GAINS_MAX_INDEX;
extern uint16_t DITHER_PATTERNS_MAX_INDEX;
extern uint16_t ZERO_POINTS_MAX_INDEX;
extern uint16_t EDGE_RATIOS_MAX_INDEX;
extern uint16_t VOLTAGE_REFS_MAX_INDEX;
extern uint16_t EDGE_OPERATIONS_MAX_INDEX;

uint16_t GET_EXPOSURES(uint8_t index) BANKED;

 uint8_t * gains_get_caption           (uint8_t * caption, uint8_t index) BANKED;
 uint8_t * dither_patterns_get_caption (uint8_t * caption, uint8_t index) BANKED;
 uint8_t * zero_points_get_caption     (uint8_t * caption, uint8_t index) BANKED;
 uint8_t * voltage_refs_get_caption    (uint8_t * caption, uint8_t index) BANKED;
 uint8_t * edge_ratios_get_caption     (uint8_t * caption, uint8_t index) BANKED;
 uint8_t * edge_operations_get_caption (uint8_t * caption, uint8_t index) BANKED;

void RENDER_CAM_REG_EDEXOPGAIN(void) BANKED;
void RENDER_CAM_REG_EXPTIME(void) BANKED;
void RENDER_CAM_REG_EDRAINVVREF(void) BANKED;
void RENDER_CAM_REG_ZEROVOUT(void) BANKED;
void RENDER_CAM_REG_DITHERPATTERN(void) BANKED;

void RENDER_CAM_REGISTERS(void) BANKED;
void RENDER_REGS_FROM_EXPOSURE(void) BANKED;
void RENDER_EDGE_FROM_EXPOSURE(void) BANKED;

#endif