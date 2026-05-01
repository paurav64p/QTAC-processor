#include "frame_source.h"
#include "frame_codec.h"
#include <string.h>
#include <stdlib.h>

bool fs_open(FrameSource *fs, const char *path, uint8_t rows, uint8_t cols)
{
    if (fs == NULL || path == NULL || rows == 0 || cols == 0)
        return false;

    if ((uint32_t)rows * cols > FRAME_MAX_CELLS)
        return false;

    fs->file              = fopen(path, "r");
    fs->rows              = rows;
    fs->cols              = cols;
    fs->frame_id          = 0;
    fs->last_timestamp_ms = 0;

    return fs->file != NULL;
}

bool fs_next(FrameSource *fs, Frame *out)
{
    if (fs == NULL || fs->file == NULL || out == NULL)
        return false;

    uint32_t num_cells = (uint32_t)fs->rows * fs->cols;
    float cells[FRAME_MAX_CELLS];
    char  line[4096];

    /* Read one line from the CSV */
    if (fgets(line, sizeof(line), fs->file) == NULL)
        return false;

    /* Skip blank lines and comment lines starting with # */
    if (line[0] == '\n' || line[0] == '\r' || line[0] == '#')
        return fs_next(fs, out);

    /* Parse comma separated floats */
    char *ptr = line;
    for (uint32_t i = 0; i < num_cells; i++) {
        char *end;
        cells[i] = strtof(ptr, &end);
        if (ptr == end)
            return false;   /* Conversion failed */
        ptr = end;
        if (*ptr == ',') ptr++;
    }

    /* Timestamp advances by 10ms per frame (100 Hz) */
    uint32_t timestamp_ms = fs->last_timestamp_ms + 10;

    CodecResult result = frame_parse(out,
                                     fs->frame_id,
                                     timestamp_ms,
                                     fs->last_timestamp_ms,
                                     fs->rows,
                                     fs->cols,
                                     cells);
    if (result != CODEC_OK)
        return false;

    fs->frame_id++;
    fs->last_timestamp_ms = timestamp_ms;
    return true;
}

void fs_close(FrameSource *fs)
{
    if (fs && fs->file) {
        fclose(fs->file);
        fs->file = NULL;
    }
}