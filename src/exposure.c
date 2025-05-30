#pragma bank 255

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "compat.h"
#include "gbcamera.h"
#include "bankdata.h"
#include "math.h"

#include "globals.h"
#include "state_camera.h"

// dither patterns
#include "dither_patterns.h"



BANKREF(exposure)

static const uint16_t exposures[] = {
    TO_EXPOSURE_VALUE(256),     TO_EXPOSURE_VALUE(272),     TO_EXPOSURE_VALUE(304),     TO_EXPOSURE_VALUE(352),
    TO_EXPOSURE_VALUE(400),     TO_EXPOSURE_VALUE(464),     TO_EXPOSURE_VALUE(512),     TO_EXPOSURE_VALUE(560),
    TO_EXPOSURE_VALUE(608),     TO_EXPOSURE_VALUE(704),     TO_EXPOSURE_VALUE(800),     TO_EXPOSURE_VALUE(912),
    TO_EXPOSURE_VALUE(1008),    TO_EXPOSURE_VALUE(1136),    TO_EXPOSURE_VALUE(1264),    TO_EXPOSURE_VALUE(1376),
    TO_EXPOSURE_VALUE(1504),    TO_EXPOSURE_VALUE(1744),    TO_EXPOSURE_VALUE(2000),    TO_EXPOSURE_VALUE(2256),
    TO_EXPOSURE_VALUE(2512),    TO_EXPOSURE_VALUE(2752),    TO_EXPOSURE_VALUE(3008),    TO_EXPOSURE_VALUE(3504),
    TO_EXPOSURE_VALUE(4000),    TO_EXPOSURE_VALUE(4496),    TO_EXPOSURE_VALUE(5008),    TO_EXPOSURE_VALUE(5504),
    TO_EXPOSURE_VALUE(6000),    TO_EXPOSURE_VALUE(7008),    TO_EXPOSURE_VALUE(8000),    TO_EXPOSURE_VALUE(9008),
    TO_EXPOSURE_VALUE(10000),   TO_EXPOSURE_VALUE(11264),   TO_EXPOSURE_VALUE(12512),   TO_EXPOSURE_VALUE(13760),
    TO_EXPOSURE_VALUE(15008),   TO_EXPOSURE_VALUE(17504),   TO_EXPOSURE_VALUE(20000),   TO_EXPOSURE_VALUE(22496),
    TO_EXPOSURE_VALUE(25008),   TO_EXPOSURE_VALUE(27504),   TO_EXPOSURE_VALUE(30000),   TO_EXPOSURE_VALUE(35008),
    TO_EXPOSURE_VALUE(40000),   TO_EXPOSURE_VALUE(45008),   TO_EXPOSURE_VALUE(50000),   TO_EXPOSURE_VALUE(55008),
    TO_EXPOSURE_VALUE(60000),   TO_EXPOSURE_VALUE(65008),   TO_EXPOSURE_VALUE(70000),   TO_EXPOSURE_VALUE(75008),
    TO_EXPOSURE_VALUE(80000),   TO_EXPOSURE_VALUE(90000),   TO_EXPOSURE_VALUE(100000),  TO_EXPOSURE_VALUE(112496),
    TO_EXPOSURE_VALUE(125008),  TO_EXPOSURE_VALUE(142496),  TO_EXPOSURE_VALUE(160000),  TO_EXPOSURE_VALUE(180000),
    TO_EXPOSURE_VALUE(200000),  TO_EXPOSURE_VALUE(225008),  TO_EXPOSURE_VALUE(250000),  TO_EXPOSURE_VALUE(275008),
    TO_EXPOSURE_VALUE(300000),  TO_EXPOSURE_VALUE(350000),  TO_EXPOSURE_VALUE(400000),  TO_EXPOSURE_VALUE(450000),
    TO_EXPOSURE_VALUE(500000),  TO_EXPOSURE_VALUE(550000),  TO_EXPOSURE_VALUE(600000),  TO_EXPOSURE_VALUE(700000),
    TO_EXPOSURE_VALUE(800000),  TO_EXPOSURE_VALUE(900000),  TO_EXPOSURE_VALUE(1000000), TO_EXPOSURE_VALUE(1048560)
};



static const table_value_t gains[] = {
    { CAM01_GAIN_140, "14.0" }, { CAM01_GAIN_155, "15.5" }, { CAM01_GAIN_170, "17.0" }, { CAM01_GAIN_185, "18.5" },
    { CAM01_GAIN_200, "20.0" }, { CAM01_GAIN_215, "21.5" }, { CAM01_GAIN_230, "23.0" }, { CAM01_GAIN_245, "24.5" },
    { CAM01_GAIN_260, "26.0" }, { CAM01_GAIN_275, "27.5" }, { CAM01_GAIN_290, "29.0" }, { CAM01_GAIN_305, "30.5" },
    { CAM01_GAIN_320, "32.0" }, { CAM01_GAIN_350, "35.0" }, { CAM01_GAIN_380, "38.0" }, { CAM01_GAIN_410, "41.0" },
    { CAM01_GAIN_440, "44.0" }, { CAM01_GAIN_455, "45.5" }, { CAM01_GAIN_470, "47.0" }, { CAM01_GAIN_515, "51.5" },
    { CAM01_GAIN_575, "57.5" }
};


static const table_value_t dither_patterns[N_DITHER_TYPES] = {
    [dither_type_Off]        = { dither_type_Off        , "Off"  },
    [dither_type_Default]    = { dither_type_Default    , "Def"  },
    [dither_type_2x2]        = { dither_type_2x2        , "2x2"  },
    [dither_type_Grid]       = { dither_type_Grid       , "Grid" },
    [dither_type_Maze]       = { dither_type_Maze       , "Maze" },
    [dither_type_Nest]       = { dither_type_Nest       , "Nest" },
    [dither_type_Fuzz]       = { dither_type_Fuzz       , "Fuzz" },
    [dither_type_Vertical]   = { dither_type_Vertical   , "Vert" },
    [dither_type_Horizonral] = { dither_type_Horizonral , "Hori" },
    [dither_type_Mix]        = { dither_type_Mix        , "Mix"  }
};
static const table_value_t zero_points[] = {
    { CAM05_ZERO_DIS, "None" }, { CAM05_ZERO_POS, "Positv" }, { CAM05_ZERO_NEG, "Negtv" }
};
static const table_value_t edge_ratios[] = {
    { CAM04_EDGE_RATIO_050, "50%" }, { CAM04_EDGE_RATIO_075, "75%" }, { CAM04_EDGE_RATIO_100, "100%" },{ CAM04_EDGE_RATIO_125, "125%" },
    { CAM04_EDGE_RATIO_200, "200%" },{ CAM04_EDGE_RATIO_300, "300%" },{ CAM04_EDGE_RATIO_400, "400%" },{ CAM04_EDGE_RATIO_500, "500%" },
};
static const table_value_t voltage_refs[] = {
    { CAM04_VOLTAGE_REF_00, "0.0" }, { CAM04_VOLTAGE_REF_05, "0.5" }, { CAM04_VOLTAGE_REF_10, "1.0" }, { CAM04_VOLTAGE_REF_15, "1.5" },
    { CAM04_VOLTAGE_REF_20, "2.0" }, { CAM04_VOLTAGE_REF_25, "2.5" }, { CAM04_VOLTAGE_REF_30, "3.0" }, { CAM04_VOLTAGE_REF_35, "3.5" },
};
static const table_value_t edge_operations[] = {
    { CAM01_EDGEOP_2D, "2D" }, { CAM01_EDGEOP_HORIZ, "Horiz" }, { CAM01_EDGEOP_VERT, "Vert" },{ CAM01_EDGEOP_NONE, "None" }
};

uint16_t EXPOSURES_MAX_INDEX       = MAX_INDEX(exposures);
uint16_t GAINS_MAX_INDEX           = MAX_INDEX(gains);
uint16_t DITHER_PATTERNS_MAX_INDEX = MAX_INDEX(dither_patterns);
uint16_t ZERO_POINTS_MAX_INDEX     = MAX_INDEX(zero_points);
uint16_t EDGE_RATIOS_MAX_INDEX     = MAX_INDEX(edge_ratios);
uint16_t VOLTAGE_REFS_MAX_INDEX    = MAX_INDEX(voltage_refs);
uint16_t EDGE_OPERATIONS_MAX_INDEX = MAX_INDEX(edge_operations);


uint16_t GET_EXPOSURES(uint8_t index) BANKED {
    return exposures[index];
}

// Can't return a pointer to the caption data since it's bank won't remain active
uint8_t * gains_get_caption           (uint8_t * caption, uint8_t index) BANKED { strcpy(caption, gains[index].caption);           return caption;}
uint8_t * dither_patterns_get_caption (uint8_t * caption, uint8_t index) BANKED { strcpy(caption, dither_patterns[index].caption); return caption;}
uint8_t * zero_points_get_caption     (uint8_t * caption, uint8_t index) BANKED { strcpy(caption, zero_points[index].caption);     return caption;}
uint8_t * voltage_refs_get_caption    (uint8_t * caption, uint8_t index) BANKED { strcpy(caption, voltage_refs[index].caption);    return caption;}
uint8_t * edge_ratios_get_caption     (uint8_t * caption, uint8_t index) BANKED { strcpy(caption, edge_ratios[index].caption);     return caption;}
uint8_t * edge_operations_get_caption (uint8_t * caption, uint8_t index) BANKED { strcpy(caption, edge_operations[index].caption); return caption;}


void RENDER_CAM_REG_EDEXOPGAIN(void)    BANKED { CAM_REG_EDEXOPGAIN  = SHADOW.CAM_REG_EDEXOPGAIN  = ((SETTING(edge_exclusive)) ? CAM01F_EDGEEXCL_V_ON : CAM01F_EDGEEXCL_V_OFF) | edge_operations[SETTING(edge_operation)].value | gains[SETTING(current_gain)].value; }
void RENDER_CAM_REG_EXPTIME(void)       BANKED { CAM_REG_EXPTIME     = SHADOW.CAM_REG_EXPTIME     = swap_bytes(SETTING(current_exposure)); }
void RENDER_CAM_REG_EDRAINVVREF(void)   BANKED { CAM_REG_EDRAINVVREF = SHADOW.CAM_REG_EDRAINVVREF = edge_ratios[SETTING(current_edge_ratio)].value | ((SETTING(invertOutput)) ? CAM04F_INV : CAM04F_POS) | voltage_refs[SETTING(current_voltage_ref)].value; }
void RENDER_CAM_REG_ZEROVOUT(void)      BANKED { CAM_REG_ZEROVOUT    = SHADOW.CAM_REG_ZEROVOUT    = zero_points[SETTING(current_zero_point)].value | TO_VOLTAGE_OUT(SETTING(voltage_out)); }
void RENDER_CAM_REG_DITHERPATTERN(void) BANKED { dither_pattern_apply(SETTING(dithering), SETTING(ditheringHighLight), SETTING(current_contrast) - 1); }

void RENDER_CAM_REGISTERS(void) BANKED {
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_EDRAINVVREF();
    RENDER_CAM_REG_ZEROVOUT();
    RENDER_CAM_REG_DITHERPATTERN();
}

void RENDER_REGS_FROM_EXPOSURE(void) BANKED {
    // Gain 14.0dB or 0 | vRef +64 mV | Horizontal edge mode | Exposure time range from  0.5ms to 0.3ms
    // Gain 14.0dB or 0 | vRef +160 mV| 2-D edge mode        | Exposure time range from   67ms to 0.8ms
    // Gain 20.0dB or 4 | vRef +96 mV | 2-D edge mode        | Exposure time range from  282ms to  32ms
    // Gain 26.0dB or 8 | vRef -192 mV| 2-D edge mode        | Exposure time range from  573ms to 164ms
    // Gain 32.0dB or 10| vRef -416 mV| No edge Operation    | Exposure time range from 1048ms to 394ms
    bool apply_dither;
    uint16_t exposure = SETTING(current_exposure);
    if (_is_CPU_FAST) {
        if (exposure < TO_EXPOSURE_VALUE(1536)) {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
            SETTING(voltage_out)        = 64;
            SETTING(current_gain)       = 0;        // CAM01_GAIN_140
            if (apply_dither = (SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = false;// dither HIGH
        } else if (exposure < TO_EXPOSURE_VALUE(64000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = 160;
            SETTING(current_gain)       = 0;        // CAM01_GAIN_140
            if (apply_dither = (SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true;// dither LOW
        } else if (exposure < TO_EXPOSURE_VALUE(564000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = 96;
            SETTING(current_gain)       = 4;        // CAM01_GAIN_200
            if (apply_dither = (!SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true; // dither LOW
        } else {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = -192;
            SETTING(current_gain)       = 8;        // CAM01_GAIN_260
            if (apply_dither = (!SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true; // dither LOW
        }
    } else {
        if (exposure < TO_EXPOSURE_VALUE(768)) {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
            SETTING(voltage_out)        = 64;
            SETTING(current_gain)       = 0;        // CAM01_GAIN_140
            if (apply_dither = (SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = false;// dither HIGH
        } else if (exposure < TO_EXPOSURE_VALUE(32000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = 160;
            SETTING(current_gain)       = 0;        // CAM01_GAIN_140
            if (apply_dither = (SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true;// dither LOW
        } else if (exposure < TO_EXPOSURE_VALUE(282000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = 96;
            SETTING(current_gain)       = 4;        // CAM01_GAIN_200
            if (apply_dither = (!SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true; // dither LOW
        } else if (exposure < TO_EXPOSURE_VALUE(573000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
            SETTING(voltage_out)        = -192;
            SETTING(current_gain)       = 8;        // CAM01_GAIN_260
            if (apply_dither = (!SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true; // dither LOW
        } else {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 3;        // CAM01_EDGEOP_NONE
            SETTING(voltage_out)        = -416;
            SETTING(current_gain)       = 10;       // CAM01_GAIN_32
            if (apply_dither = (!SETTING(ditheringHighLight)))
                SETTING(ditheringHighLight) = true; // dither LOW
        }
    }
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
    RENDER_CAM_REG_ZEROVOUT();
    if (apply_dither) RENDER_CAM_REG_DITHERPATTERN();
}

void RENDER_EDGE_FROM_EXPOSURE(void) BANKED {
    uint16_t exposure = SETTING(current_exposure);
    if (_is_CPU_FAST) {
        if (exposure < TO_EXPOSURE_VALUE(1536)) {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
        } else {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
        }
    } else {
        if (exposure < TO_EXPOSURE_VALUE(768)) {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 1;        // CAM01_EDGEOP_HORIZ
        } else if (exposure < TO_EXPOSURE_VALUE(573000)) {
            SETTING(edge_exclusive)     = true;     // CAM01F_EDGEEXCL_V_ON
            SETTING(edge_operation)     = 0;        // CAM01_EDGEOP_2D
        } else {
            SETTING(edge_exclusive)     = false;    // CAM01F_EDGEEXCL_V_OFF
            SETTING(edge_operation)     = 3;        // CAM01_EDGEOP_NONE
        }
    }
    CAMERA_SWITCH_RAM(CAMERA_BANK_REGISTERS);
    RENDER_CAM_REG_EDEXOPGAIN();
    RENDER_CAM_REG_EXPTIME();
}
