#ifndef FRAME_CODEC_H
#define FRAME_CODEC_H

#include "ring_buffer.h"
#include <stdbool.h>

/* Return codes for frame parsing */
typedef enum {
    CODEC_OK = 0,
    CODEC_ERR_NULL,          /* NULL pointer passed in        */
    CODEC_ERR_BAD_DIMS,      /* rows or cols are zero         */
    CODEC_ERR_TOO_LARGE,     /* rows*cols exceeds FRAME_MAX_CELLS */
    CODEC_ERR_BAD_VALUES,    /* cell values out of range      */
    CODEC_ERR_BAD_TIMESTAMP  /* timestamp went backwards      */
} CodecResult;

/* Parse and validate a raw frame
 * Populates out on success, leaves it unchanged on failure
 * prev_timestamp_ms: pass 0 for the very first frame */
CodecResult frame_parse(Frame *out,
                        uint32_t frame_id,
                        uint32_t timestamp_ms,
                        uint32_t prev_timestamp_ms,
                        uint8_t  rows,
                        uint8_t  cols,
                        const float *cells);

/* Human readable string for a CodecResult */
const char *codec_result_str(CodecResult result);

#endif /* FRAME_CODEC_H */