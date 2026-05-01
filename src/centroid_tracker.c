#include "centroid_tracker.h"
#include <string.h>
#include <math.h>

void ct_init(CentroidTracker *ct)
{
    memset(ct->history, 0, sizeof(ct->history));
    ct->count = 0;
    ct->index = 0;
}

bool ct_update(CentroidTracker *ct, const Frame *frame, Centroid *out)
{
    float weighted_row = 0.0f;
    float weighted_col = 0.0f;
    float total        = 0.0f;

    for (uint8_t r = 0; r < frame->rows; r++) {
        for (uint8_t c = 0; c < frame->cols; c++) {
            float val = frame->cells[r * frame->cols + c];
            weighted_row += val * (float)r;
            weighted_col += val * (float)c;
            total        += val;
        }
    }

    /* Nothing touching - centroid undefined */
    if (total == 0.0f)
        return false;

    Centroid centroid;
    centroid.row = weighted_row / total;
    centroid.col = weighted_col / total;

    /* Store in circular history buffer */
    ct->history[ct->index] = centroid;
    ct->index = (ct->index + 1) % CENTROID_HISTORY;
    if (ct->count < CENTROID_HISTORY)
        ct->count++;

    if (out)
        *out = centroid;

    return true;
}

bool ct_latest(const CentroidTracker *ct, Centroid *out)
{
    if (ct->count == 0)
        return false;

    uint32_t last = (ct->index + CENTROID_HISTORY - 1) % CENTROID_HISTORY;
    *out = ct->history[last];
    return true;
}

float ct_displacement(const CentroidTracker *ct, uint32_t n_frames)
{
    if (ct->count < 2)
        return 0.0f;

    if (n_frames > ct->count)
        n_frames = ct->count;

    /* Oldest and newest within requested window */
    uint32_t newest = (ct->index + CENTROID_HISTORY - 1) % CENTROID_HISTORY;
    uint32_t oldest = (ct->index + CENTROID_HISTORY - n_frames) % CENTROID_HISTORY;

    float dr = ct->history[newest].row - ct->history[oldest].row;
    float dc = ct->history[newest].col - ct->history[oldest].col;

    return sqrtf(dr * dr + dc * dc);
}
