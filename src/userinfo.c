#pragma bank 255

#include <gbdk/platform.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "userinfo.h"
#include "protected.h"

static const uint8_t digits_array[] = "00123456789?????";

static const uint8_t character_array[] = {
    ' ', ' ', ' ', ' ', ' ', ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '_', '\\',',', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    ' ', ' ', ' ', '-', '&', '!', '?', ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '•', '~', '?', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '/', ':', '~', '"', '@', ' ', ' ', ' ', ' ', ' ', ' ', ' '
};

uint8_t * userinfo_get_userid(uint8_t * buf) BANKED {
    SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    strcpy(buf, "GB-");
    uint8_t *dest = buf + 3;
    for (uint8_t i = 0, *src = &cam_owner_data.user_id; i != 4; i++, src++) {
        *dest++ = digits_array[(*src >> 4) & 0x0f];
        *dest++ = digits_array[(*src & 0x0f)];
    }
    *dest = 0;
    return buf;
}

uint8_t * userinfo_get_username(uint8_t * buf) BANKED {
    SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    uint8_t *dest = buf;
    for (uint8_t i = 0, *src = cam_owner_data.user_name; i != sizeof(cam_owner_data.user_name); i++, src++) {
        *dest++ = ((*src < 0x50) || (*src > 0xdf)) ? ' ' : character_array[(*src - 0x50)];
    }
    *dest = 0;
    return buf;
}

uint8_t * userinfo_get_birthdate(uint8_t * buf) BANKED {
    SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    if (cam_owner_data.datepart1) {
        buf[0]  = digits_array[(cam_owner_data.datepart1 >> 4) & 0x0f];
        buf[1]  = digits_array[(cam_owner_data.datepart1 & 0x0f)];
        buf[2]  = '/';
    } else strcpy(buf, "--/");
    if (cam_owner_data.datepart2) {
        buf[3]  = digits_array[(cam_owner_data.datepart2 >> 4) & 0x0f];
        buf[4]  = digits_array[(cam_owner_data.datepart2 & 0x0f)];
        buf[5]  = '/';
    } else strcpy(buf + 3, "--/");
    if (cam_owner_data.year) {
        buf[6]  = digits_array[(cam_owner_data.year >> 4) & 0x0f];
        buf[7]  = digits_array[(cam_owner_data.year & 0x0f)];
        buf[8]  = digits_array[(cam_owner_data.year >> 12) & 0x0f];
        buf[9]  = digits_array[(cam_owner_data.year >> 8) & 0x0f];
        buf[10] = 0x00;
    } else strcpy(buf + 6, "----");
    return buf;
}

uint8_t userinfo_get_gender(void) BANKED {
    SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    return ((cam_owner_data.gender_blood & 0x03) != 0x03) ? cam_owner_data.gender_blood & 0x03 : 0;
}
