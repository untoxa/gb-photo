#ifndef _COMPAT_H_INCLUDE
#define _COMPAT_H_INCLUDE

#include <gbdk/platform.h>

// Pic-n-Rec compatible version of vsync()
void sync_vblank(void);

#ifndef S_PAL
#define S_PAL(n) (n)
#endif

#if defined(NINTENDO)
__BYTE_REG rRAMG_MBC5;
__BYTE_REG rROMB0_MBC5;
__BYTE_REG rROMB1_MBC5;
__BYTE_REG rRAMB_MBC5;
#define CAMERA_ENABLE_RAM (rRAMG_MBC5 = 0x0A)
#define CAMERA_DISABLE_RAM (rRAMG_MBC5 = 0x00)
#define CAMERA_SWITCH_RAM(b) (rRAMB_MBC5 = (b))
#define CAMERA_SWITCH_ROM(b) (_current_bank = (b), rROMB0_MBC5 = (b))
#else
#define CAMERA_ENABLE_RAM ENABLE_RAM
#define CAMERA_DISABLE_RAM DISABLE_RAM
#define CAMERA_SWITCH_RAM(b) SWITCH_RAM(b)
#define CAMERA_SWITCH_ROM(b) SWITCH_ROM(b)
#endif

#endif