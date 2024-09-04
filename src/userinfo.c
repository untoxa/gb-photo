#pragma bank 255

#include <gbdk/platform.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "compat.h"
#include "userinfo.h"
#include "protected.h"

static const uint8_t digits_array[] = "00123456789?????";

static const uint8_t character_array[] = {
    ' ', ' ', ' ', ' ', ' ', ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '_', '\\',',', '.',0xc1,0xc2,0xc0,0xc4,0xc9,0xca,0xc8,0xcb,0xcd,0xcf,0xd3,0xd6,
    0xda,0xdc,0xd1,'-', '&', '!', '?', ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', 0xb7, '~',0xbf,' ',0xe1,0xe2,0xe0,0xe4,0xe9,0xea,0xe8,0xeb,0xed,0xef,
    0xf3,0xf6,0xfa,0xfc,0xf1,0xe7,0xdf,0xa9,0xae,0xd7,'0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '/', ':', '~', '"', '@', ' ', ' ', ' ', ' ', ' ', ' ', ' '
};

uint8_t * userinfo_get_userid(uint8_t * buf) BANKED {
    CAMERA_SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    strcpy(buf, "GB-");
    uint8_t *dest = buf + 3;
    for (uint8_t i = 0, *src = &cam_owner_data.user_info.user_id; i != 4; i++, src++) {
        *dest++ = digits_array[(*src >> 4) & 0x0f];
        *dest++ = digits_array[(*src & 0x0f)];
    }
    *dest = 0;
    return buf;
}

uint8_t * userinfo_get_username(uint8_t * buf) BANKED {
    CAMERA_SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    uint8_t *dest = buf;
    for (uint8_t i = 0, *src = cam_owner_data.user_info.user_name; i != sizeof(cam_owner_data.user_info.user_name); i++, src++) {
        *dest++ = ((*src < 0x50) || (*src > 0xdf)) ? ' ' : character_array[(*src - 0x50)];
    }
    *dest = 0;
    return buf;
}

uint8_t * userinfo_get_birthdate(uint8_t * buf) BANKED {
    CAMERA_SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    if (cam_owner_data.user_info.datepart1) {
        buf[0]  = digits_array[(cam_owner_data.user_info.datepart1 >> 4) & 0x0f];
        buf[1]  = digits_array[(cam_owner_data.user_info.datepart1 & 0x0f)];
        buf[2]  = '/';
    } else strcpy(buf, "--/");
    if (cam_owner_data.user_info.datepart2) {
        buf[3]  = digits_array[(cam_owner_data.user_info.datepart2 >> 4) & 0x0f];
        buf[4]  = digits_array[(cam_owner_data.user_info.datepart2 & 0x0f)];
        buf[5]  = '/';
    } else strcpy(buf + 3, "--/");
    if (cam_owner_data.user_info.year) {
        buf[6]  = digits_array[(cam_owner_data.user_info.year >> 4) & 0x0f];
        buf[7]  = digits_array[(cam_owner_data.user_info.year & 0x0f)];
        buf[8]  = digits_array[(cam_owner_data.user_info.year >> 12) & 0x0f];
        buf[9]  = digits_array[(cam_owner_data.user_info.year >> 8) & 0x0f];
        buf[10] = 0x00;
    } else strcpy(buf + 6, "----");
    return buf;
}

uint8_t userinfo_get_gender(void) BANKED {
    CAMERA_SWITCH_RAM(CAMERA_BANK_OWNER_DATA);
    return ((cam_owner_data.user_info.gender_blood & 0x03) != 0x03) ? cam_owner_data.user_info.gender_blood & 0x03 : 0;
}
