#include "frame_codec.h"
#include <string.h>

/* Pressure values must be in this range (normalised 0.0 to 1.0) */
#define CELL_MIN 0.0f
#define CELL_MAX 1.0f

CodecResult frame_parse(Frame *out,
                        uint32_t frame_id,
                        uint32_t timestamp_ms,
                        uint32_t prev_timestamp_ms,
                        uint8_t  rows,
                        uint8_t  cols,
                        const float *cells)
{
    if (out == NULL || cells == NULL)
        return CODEC_ERR_NULL;

    if (rows == 0 || cols == 0)
        return CODEC_ERR_BAD_DIMS;

    if ((uint32_t)rows * cols > FRAME_MAX_CELLS)
        return CODEC_ERR_TOO_LARGE;

    /* Allow equal timestamps on the very first frame (prev == 0) */
    if (prev_timestamp_ms > 0 && timestamp_ms <= prev_timestamp_ms)
        return CODEC_ERR_BAD_TIMESTAMP;

    uint32_t num_cells = (uint32_t)rows * cols;
    for (uint32_t i = 0; i < num_cells; i++) {
        if (cells[i] < CELL_MIN || cells[i] > CELL_MAX)
            return CODEC_ERR_BAD_VALUES;
    }

    out->frame_id     = frame_id;
    out->timestamp_ms = timestamp_ms;
    out->rows         = rows;
    out->cols         = cols;
    memcpy(out->cells, cells, num_cells * sizeof(float));

    return CODEC_OK;
}

const char *codec_result_str(CodecResult result)
{
    switch (result) {
        case CODEC_OK:              return "OK";
        case CODEC_ERR_NULL:        return "ERR_NULL";
        case CODEC_ERR_BAD_DIMS:    return "ERR_BAD_DIMS";
        case CODEC_ERR_TOO_LARGE:   return "ERR_TOO_LARGE";
        case CODEC_ERR_BAD_VALUES:  return "ERR_BAD_VALUES";
        case CODEC_ERR_BAD_TIMESTAMP: return "ERR_BAD_TIMESTAMP";
        default:                    return "ERR_UNKNOWN";
    }
}