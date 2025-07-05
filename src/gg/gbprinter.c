#pragma bank 255

#include <gbdk/platform.h>
#include <stdint.h>

#include "compat.h"
#include "gbprinter.h"

#include "state_camera.h"

uint8_t printer_completion;
far_ptr_t printer_progress_handler;

start_print_pkt_t PRN_PKT_START;

uint8_t gbprinter_detect(uint8_t delay) BANKED {
    delay;
    return PRN_STATUS_ER0;
}

uint8_t gbprinter_print_image(const uint8_t * image, uint8_t image_bank, const frame_desc_t * frame, uint8_t frame_bank) BANKED {
    image; image_bank; frame; frame_bank;
    return PRN_STATUS_ER0;
}

uint8_t gbprinter_print_screen_rect(uint8_t sx, uint8_t sy, uint8_t sw, uint8_t sh, uint8_t centered) BANKED {
    sx; sy; sw; sh; centered;
    return PRN_STATUS_ER0;
}