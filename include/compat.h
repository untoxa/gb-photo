#ifndef _COMPAT_H_INCLUDE
#define _COMPAT_H_INCLUDE

#include <gbdk/platform.h>

// Pic-n-Rec compatible version of vsync()
#if defined(NINTENDO)
void sync_vblank(void) PRESERVES_REGS(b, c, d, e);
#else
#define sync_vblank vsync
#endif

#ifndef S_PAL
#define S_PAL(n) (n)
#endif

#endif