#ifndef _COMPAT_H_INCLUDE
#define _COMPAT_H_INCLUDE

#include <gbdk/platform.h>

#if defined(SEGA)
#if defined(MASTERSYSTEM)
#define DMG_BLACK     0x01u
#define DMG_DARK_GRAY 0x0cu
#define DMG_LITE_GRAY 0x0eu
#define DMG_WHITE     0x0fu
#elif defined(GAMEGEAR)
#define DMG_BLACK     0x03u
#define DMG_DARK_GRAY 0x02u
#define DMG_LITE_GRAY 0x01u
#define DMG_WHITE     0x00u
#endif
#endif

#ifndef S_PAL
#define S_PAL(n) (n)
#endif

__BYTE_REG rRAMG_MBC5;
__BYTE_REG rROMB0_MBC5;
__BYTE_REG rROMB1_MBC5;
__BYTE_REG rRAMB_MBC5;

#if defined(NINTENDO)

// Pic-n-Rec compatible version of vsync()
void sync_vblank(void);

__REG _current_rom;
#define CURRENT_ROM_BANK _current_rom
#define CAMERA_ENABLE_RAM (rRAMG_MBC5 = 0x0A)
#define CAMERA_DISABLE_RAM (rRAMG_MBC5 = 0x00)
#define CAMERA_SWITCH_RAM(b) (rRAMB_MBC5 = (b))
#define CAMERA_SWITCH_ROM(b) (CURRENT_ROM_BANK = (b), rROMB0_MBC5 = (b))

#elif defined(SEGA)

// link cable definitions
// DPC0 = MISO
// DPC1 = MOSI
// DPC6 = CLK

#ifndef GGEXT_NINT
#define GGEXT_NINT      0b10000000
#endif

#define LINK_PIN_CLK    6
#define LINK_PIN_MOSI   1
#define LINK_PIN_MISO   0
#define LINK_CLK        0b01000000
#define LINK_MOSI       0b00000010
#define LINK_MISO       0b00000001

#define LINK_SLAVE      0b01000001
#define LINK_MASTER     0b10000001

__BYTE_REG _current_rom;
#define CURRENT_ROM_BANK _current_rom
#define CAMERA_ENABLE_RAM (rRAMG_MBC5 = 0x0A)
#define CAMERA_DISABLE_RAM (rRAMG_MBC5 = 0x00)
#define CAMERA_SWITCH_RAM(b) (rRAMB_MBC5 = (b))
#define CAMERA_SWITCH_ROM(b) (CURRENT_ROM_BANK = (b), rROMB0_MBC5 = (b))
//#define CAMERA_EMULATE_CAPTURE

inline void sync_vblank(void) {
    vsync();
}

#endif


#endif