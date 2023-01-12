#ifndef _COMPAT_H_INCLUDE
#define _COMPAT_H_INCLUDE

#include <gbdk/platform.h>

#ifndef vsync
#define vsync wait_vbl_done
#endif

#endif