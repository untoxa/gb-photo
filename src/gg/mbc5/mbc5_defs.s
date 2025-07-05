_rRAMG_MBC5   = 0x0000
_rROMB0_MBC5  = 0x2000
_rROMB1_MBC5  = 0x3000
_rRAMB_MBC5   = 0x4000

.globl _rRAMG_MBC5, _rROMB0_MBC5, _rROMB1_MBC5, _rRAMB_MBC5

        .area _DATA

__current_rom::
        .ds 0x01
