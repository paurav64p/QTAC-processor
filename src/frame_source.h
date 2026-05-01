#ifndef FRAME_SOURCE_H
#define FRAME_SOURCE_H

#include "ring_buffer.h"
#include <stdio.h>
#include <stdbool.h>

/* Holds state for reading frames from a CSV file */
typedef struct {
    FILE    *file;
    uint8_t  rows;
    uint8_t  cols;
    uint32_t frame_id;
    uint32_t last_timestamp_ms;
} FrameSource;

/* Open a CSV file for reading
 * rows and cols define the grid dimensions expected per frame
 * Returns true on success */
bool fs_open(FrameSource *fs, const char *path, uint8_t rows, uint8_t cols);

/* Read the next frame from the CSV into out
 * Returns true if a frame was read, false on EOF or error */
bool fs_next(FrameSource *fs, Frame *out);

/* Close the file */
void fs_close(FrameSource *fs);

#endif /* FRAME_SOURCE_H */