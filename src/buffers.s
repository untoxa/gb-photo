.title  "Frame buffer"
.module frame_buffer
.area   _FRAME_BUFFER (ABS)

.org    0xC000

_frame_buffer::
    .ds 0x0E00
