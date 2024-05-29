#include "gbdk/platform.h"
#include "gbcamera.h"
#include "vwf.h"
#include "screen.h"
#include "state_camera.h"
#include "mode_slitscan.h"

#include <string.h>

#pragma bank 255

// TODO: OPTIONAL: programatic exposure stepping

#define BYTES_PER_TILE_ROW       2u
#define BYTES_PER_TILE           (DEVICE_TILE_HEIGHT * BYTES_PER_TILE_ROW)

#define DEVICE_TILE_HEIGHT       8u
#define SLITSCAN_CAM_LINE_START  0u
#define SLITSCAN_CAM_LINE_START_SINGLE_LINE  ((CAMERA_IMAGE_TILE_HEIGHT * DEVICE_TILE_HEIGHT) / 2u)
#define SLITSCAN_DEST_LINE_START 0u
#define SLITSCAN_DEST_LINE_DONE  (CAMERA_IMAGE_TILE_HEIGHT * DEVICE_TILE_HEIGHT) // Basically last scanline in image + 1
#define SLITSCAN_CAM_ADDR_START_SINGLE_LINE ((CAMERA_IMAGE_TILE_HEIGHT / 2)) * (BYTES_PER_TILE * CAMERA_IMAGE_TILE_WIDTH) // Start of middle vertical tile row of camera image

#define LINE_IS_NEW_TILE(y_line) ((y_line & 0x07u) == 0x00u)
#define CAMERA_NEXT_TILE_ROW_FROM_END_OF_FIRST_TILE  ((CAMERA_IMAGE_TILE_WIDTH - 1u) * BYTES_PER_TILE)
#define DISPLAY_NEXT_TILE_ROW_FROM_END_OF_FIRST_TILE ((DEVICE_SCREEN_WIDTH - 1u) * BYTES_PER_TILE)

#define DISPLAY_CAM_START_ROW        (IMAGE_DISPLAY_Y)
#define DISPLAY_CAM_COL_OFFSET       (IMAGE_DISPLAY_X)
#define DISPLAY_CAM_COL_ADDR_OFFSET  (DISPLAY_CAM_COL_OFFSET * BYTES_PER_TILE)

// Number of frames to continue capturing for after a motion trigger (provides some hysteresis)
#define SLITSCAN_MOTION_ADDITIONAL_FRAMES 3u  // TODO: maybe make adjustable

static uint8_t src_line_camera;      // Which camera scanline to read from
static uint8_t dest_line_slitscan;   // Resulting image scanline to write to
static bool slitscan_in_progress = false;

uint16_t slitscan_opt_motiontrigger;
uint8_t  slitscan_opt_delay_frames;

static uint16_t motion_additional_frames = 0;


// Pointers into camera source, slitscan output and display buffers
static const uint8_t * src_addr_camera;
static       uint8_t * dest_addr_slitscan;
static       uint8_t * dest_display_addr;

// Ref: last_seen[CAMERA_IMAGE_SIZE];
static uint8_t slitscan_picture[CAMERA_IMAGE_SIZE];

const uint8_t slitscan_delays[N_SLITSCAN_DELAYS] = {
    [slitscan_delay_none]      = SECOND_TO_FRAMES(0),
    [slitscan_delay_0_05_sec]  = SECOND_TO_FRAMES(0.05),
    [slitscan_delay_0_10_sec]  = SECOND_TO_FRAMES(0.10),
    [slitscan_delay_0_25_sec]  = SECOND_TO_FRAMES(0.25),
    [slitscan_delay_0_50_sec]  = SECOND_TO_FRAMES(0.5),
    [slitscan_delay_1_00_sec]  = SECOND_TO_FRAMES(1.0),
};

// ~Range: Min:0 ... Max:336 ((14 * 8) + ((14 * 8) << 1))
const uint16_t slitscan_motiontriggers[N_SLITSCAN_MOTIONTRIGGERS] = {
    [slitscan_MotionTrigger_none] = 0u,
    [slitscan_MotionTrigger_10]   = 10u,
    [slitscan_MotionTrigger_20]   = 20u,
    [slitscan_MotionTrigger_30]   = 30u,
    [slitscan_MotionTrigger_50]   = 50u,
    [slitscan_MotionTrigger_75]   = 75u,
    [slitscan_MotionTrigger_100]  = 100u,
    [slitscan_MotionTrigger_150]  = 150u,
    [slitscan_MotionTrigger_200]  = 200u,
    [slitscan_MotionTrigger_250]  = 250u
};

// Lookup Table for number of bits enabled in a given nybble
const uint8_t nybble_bit_count[] = {
    // 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
       0u,   1u,   1u,   2u,   1u,   2u,   2u,   3u,   1u,   2u,   2u,   3u,   2u,   3u,   3u,   4u
};



// Returns whether a slitscan capture is currently being captured
bool slitscan_is_capturing(void) NONBANKED {
    return slitscan_in_progress;
}


// Wait N frames before next scanline capture if requested
// TODO: non-blocking delay
void slitscan_check_delay(void) {

    if (slitscan_opt_delay_frames > 0) {
        uint8_t frame_count = slitscan_opt_delay_frames;
        while (frame_count--) vsync();
    }
}


static uint8_t count_diff_bits(uint8_t a, uint8_t b) {
    uint8_t temp = a ^ b;
    return nybble_bit_count[temp & 0x0Fu] + nybble_bit_count[temp >> 4];
}


// Check if a new line is over the motion threshold
static bool slitscan_check_motion_trigger(void) {

    // Motion trigger requires single scanline mode
    if (OPTION(slitscan_singleline) == false)
            return true;

    // If the feature isn't enabled then always return TRUE, found motion
    if (slitscan_opt_motiontrigger == 0)
            return true;

    const uint8_t * src_addr = src_addr_camera;  // Current scanline in camera image
    const uint8_t * ref_addr = slitscan_picture; // First scanline recorded in slitscan image is used as reference
    uint16_t line_diff_pixel_count = 0u;

    // Calculate number of different pixels for the scanline
    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);
    for (uint8_t tile_col = 0; tile_col < CAMERA_IMAGE_TILE_WIDTH; tile_col++) {
        line_diff_pixel_count += count_diff_bits(*src_addr++, *ref_addr++);
        line_diff_pixel_count += count_diff_bits(*src_addr,   *ref_addr) << 1; // Second bitplane
        // Move to next tile column
        src_addr  += DEVICE_TILE_SIZE - 1u;
        ref_addr  += DEVICE_TILE_SIZE - 1u;
    }

    // Check if it's above the difference threshold vs the last line
    // If so, reset the additional frame/line capture counter
    if (line_diff_pixel_count > slitscan_opt_motiontrigger) // SLITSCAN_MOTION_THRESHOLD)
        motion_additional_frames = SLITSCAN_MOTION_ADDITIONAL_FRAMES;

    if (motion_additional_frames > 0) {
        motion_additional_frames--;
        return true;
    } else
        return false;
}


// TODO: handle flipped mode?
// Copy single scanline row from the "last seen" image to the working slit-scan image buffer
static void slitscan_copy_and_display_line_horiz(void) {

    const uint8_t * src_addr  = src_addr_camera;
          uint8_t * dest_addr = dest_addr_slitscan;
          uint8_t * disp_addr = dest_display_addr;

    SWITCH_RAM(CAMERA_BANK_LAST_SEEN);

    // Loop through one scanline row on the source camera image copying to the scanline image
    // Two bytes per line tile entry in 2bpp mode
    for (uint8_t tile_col = 0; tile_col < CAMERA_IMAGE_TILE_WIDTH; tile_col++) {
        // First byte of tile row
        set_vram_byte(disp_addr++,  *src_addr);
        *dest_addr++ = *src_addr++;
        // First byte of tile row
        set_vram_byte(disp_addr,  *src_addr);
        *dest_addr   = *src_addr;

        // Move to next tile column
        src_addr  += DEVICE_TILE_SIZE - 1u;
        dest_addr += DEVICE_TILE_SIZE - 1u;
        disp_addr += DEVICE_TILE_SIZE - 1u;
    }
}


static void slitscan_display_picture(uint8_t * src_pic) {
    uint8_t ypos = (OPTION(camera_mode) == camera_mode_manual) ? (IMAGE_DISPLAY_Y + 1) : IMAGE_DISPLAY_Y;
    screen_load_live_image(IMAGE_DISPLAY_X, ypos, CAMERA_IMAGE_TILE_WIDTH, CAMERA_IMAGE_TILE_HEIGHT,
                           src_pic,
                           OPTION(flip_live_view),
                           ((_is_COLOR) && OPTION(enable_DMA) && !((OPTION(after_action) == after_action_picnrec) || (OPTION(after_action) == after_action_picnrec_video))));
}

// Called from state_camera main loop to cancel a capture in progress
void slitscan_mode_on_cancel(void) BANKED {
    slitscan_in_progress = false;
}


// Called from state_camera main loop once a trigger event occurs
// Resets the slit scan capture state to prepare for incoming photo data
void slitscan_mode_on_trigger(void) BANKED {

    slitscan_opt_motiontrigger = slitscan_motiontriggers[OPTION(slitscan_motiontrigger)];
    slitscan_opt_delay_frames =  slitscan_delays[OPTION(slitscan_delay)];

    // Clear slitscan image and display blank area
    memset(slitscan_picture, 0, CAMERA_IMAGE_SIZE);
    slitscan_display_picture(slitscan_picture);

    if (slitscan_opt_motiontrigger > 0) {
        motion_additional_frames = SLITSCAN_MOTION_ADDITIONAL_FRAMES;
    }

    // Reset control vars
    if (OPTION(slitscan_singleline)) {
        src_line_camera     = SLITSCAN_CAM_LINE_START_SINGLE_LINE;
        src_addr_camera     = last_seen + SLITSCAN_CAM_ADDR_START_SINGLE_LINE;
    } else {
        src_line_camera     = SLITSCAN_CAM_LINE_START;
        src_addr_camera     = last_seen;
    }

    dest_line_slitscan  = SLITSCAN_DEST_LINE_START;

    dest_addr_slitscan  = slitscan_picture;
    dest_display_addr   = (uint8_t *)screen_tile_addresses[(dest_line_slitscan / 8) + DISPLAY_CAM_START_ROW] + DISPLAY_CAM_COL_ADDR_OFFSET;
    slitscan_in_progress = true;
}


// Pre-increment image and display pointer offsets for next incoming camera scanline
// Next scanline in same tile row
inline void slitscan_handle_line_increment(void) {
    if (!OPTION(slitscan_singleline)) src_addr_camera += BYTES_PER_TILE_ROW;
    dest_addr_slitscan += BYTES_PER_TILE_ROW;
    dest_display_addr  += BYTES_PER_TILE_ROW;

    // Handle increment when it's a new tile row
    if (LINE_IS_NEW_TILE(dest_line_slitscan)) {
        if (!OPTION(slitscan_singleline)) src_addr_camera += CAMERA_NEXT_TILE_ROW_FROM_END_OF_FIRST_TILE;
        dest_addr_slitscan += CAMERA_NEXT_TILE_ROW_FROM_END_OF_FIRST_TILE;
        dest_display_addr   = (uint8_t *)screen_tile_addresses[(dest_line_slitscan / 8) + DISPLAY_CAM_START_ROW] + DISPLAY_CAM_COL_ADDR_OFFSET;
    }
}


// Called from state_camera main loop after an image has been captured
// Resets the slit scan capture state to prepare for incoming photo data
// TODO: void slitscan_mode_copy_next_line(void) {
uint8_t slitscan_mode_on_image_captured(void) BANKED {

    // If motion detection was enabled and none found then skip capturing for this frame
    if (slitscan_check_motion_trigger() == false) {
        return SLITSCAN_STATE_STILL_CAPTURING;
    }

    slitscan_copy_and_display_line_horiz();
    if (!OPTION(slitscan_singleline)) src_line_camera++;
    dest_line_slitscan++;

    if (dest_line_slitscan != SLITSCAN_DEST_LINE_DONE) {
        slitscan_handle_line_increment();
        slitscan_check_delay();
        // Request the next frame
        return SLITSCAN_STATE_STILL_CAPTURING;
    } else {
        // The slit-scan capture has completed
        // Copy resulting slit-scan image to the "last-seen" camera image buffer expected by most processing
        memcpy(last_seen, slitscan_picture, CAMERA_IMAGE_SIZE);
        slitscan_in_progress = false;
        return SLITSCAN_STATE_DONE;
    }
}
