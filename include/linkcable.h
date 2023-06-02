#ifndef __LINKCABLE_H_INCLUDE__
#define __LINKCABLE_H_INCLUDE__

uint8_t linkcable_transfer_reset(void) BANKED;
uint8_t linkcable_transfer_image(const uint8_t * image, uint8_t image_bank) BANKED;

#endif