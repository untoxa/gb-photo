#pragma bank 255

#include <gbdk/platform.h>
#include <stdbool.h>

#include "gbcamera.h"
#include "sd.h"
#include "pff.h"

bool fs_mounted  = false;
FATFS fs;       /* File system object */
DIR dir;        /* Directory object */
FILINFO fno;    /* File information */

bool lastseen_to_sd(camera_flip_e flip) BANKED {
    flip;

    if (!fs_mounted) {
        if (pf_mount(&fs) == FR_OK) {
            fs_mounted = true;
        } else {
            return false;
        }
    }

    return false;
}