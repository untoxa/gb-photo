#pragma bank 255

#include "hUGEDriver.h"
#include <stddef.h>

static const unsigned char order_cnt = 0;

const void __at(255) __bank_song;
const hUGESong_t song = {7, &order_cnt, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
