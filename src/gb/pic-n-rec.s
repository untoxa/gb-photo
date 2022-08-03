        .include        "global.s"

        .globl __current_1bpp_colors

        .area   _HEADER_PIC_N_REC (ABS)

        .org 0x60
_picnrec_trigger::
        ei
        halt
        halt
        halt
        ret
