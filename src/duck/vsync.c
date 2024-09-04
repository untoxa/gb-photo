#include <gbdk/platform.h>

#include "state_camera.h"

static SFR AT(0xFF91) vbl_done;

void sync_vblank(void) {
    if (!(LCDC_REG & LCDCF_ON)) return;
    vbl_done = 0;
    do {
        if (OPTION(cart_type) != cart_type_iG_AIO) __asm__("halt\nnop");
    } while (!vbl_done);
}
