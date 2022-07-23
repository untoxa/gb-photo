#ifndef __STATE_GALLERY_H_INCLUDE__
#define __STATE_GALLERY_H_INCLUDE__

#include <stdint.h>

#include "gbcamera.h"
#include "vector.h"

BANKREF_EXTERN(state_gallery)

VECTOR_DECLARE_EXTERN(used_slots, uint8_t, CAMERA_MAX_IMAGE_SLOTS);
VECTOR_DECLARE_EXTERN(free_slots, uint8_t, CAMERA_MAX_IMAGE_SLOTS);

inline uint8_t images_taken() {
    return VECTOR_LEN(used_slots);
}

inline uint8_t images_total() {
    return CAMERA_MAX_IMAGE_SLOTS;
}

inline uint8_t images_free() {
    return VECTOR_LEN(free_slots);
}

#endif