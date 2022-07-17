#ifndef __STATE_THUMBNAILS_H_INCLUDE__
#define __STATE_THUMBNAILS_H_INCLUDE__

#include <stdint.h>

#define THUMBNAIL_DISPLAY_X 2
#define THUMBNAIL_DISPLAY_Y 1
#define THUMBNAIL_DISPLAY_WIDTH 16
#define THUMBNAIL_DISPLAY_HEIGHT 16

#define MAX_PREVIEW_THUMBNAILS 16
#define MAX_PREVIEW_PAGES 2

#define THUMBS_COUNT_X 4
#define THUMBS_COUNT_Y 4

typedef struct thumb_coord_t {
    uint8_t x, y;
} thumb_coord_t;

#endif