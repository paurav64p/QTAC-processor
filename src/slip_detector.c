#include "slip_detector.h"
#include <string.h>
#include <math.h>

void sd_init(SlipDetector *sd,
             float gradient_threshold,
             float pressure_drop)
{
    memset(&sd->prev_frame, 0, sizeof(Frame));
    sd->initialized        = 0;
    sd->gradient_threshold = gradient_threshold;
    sd->pressure_drop      = pressure_drop;
}

/* Compute total pressure across all cells */
static float total_pressure(const Frame *frame)
{
    uint32_t num_cells = (uint32_t)frame->rows * frame->cols;
    float    sum       = 0.0f;
    for (uint32_t i = 0; i < num_cells; i++)
        sum += frame->cells[i];
    return sum;
}

/* Compute mean absolute difference between two frames cell by cell */
static float mean_gradient(const Frame *a, const Frame *b)
{
    uint32_t num_cells = (uint32_t)a->rows * a->cols;
    float    sum       = 0.0f;
    for (uint32_t i = 0; i < num_cells; i++) {
        float diff = a->cells[i] - b->cells[i];
        sum += diff < 0.0f ? -diff : diff;
    }
    return sum / (float)num_cells;
}

bool sd_process(SlipDetector *sd,
                const Frame  *frame,
                SlipEvent    *event)
{
    /* Seed on first frame - nothing to compare yet */
    if (!sd->initialized) {
        memcpy(&sd->prev_frame, frame, sizeof(Frame));
        sd->initialized = 1;
        return false;
    }

    float grad     = mean_gradient(frame, &sd->prev_frame);
    float prev_tot = total_pressure(&sd->prev_frame);
    float curr_tot = total_pressure(frame);

    /* Only evaluate slip if there was meaningful pressure previously */
    bool slip = false;
    if (prev_tot > 0.0f) {
        float drop_fraction = (prev_tot - curr_tot) / prev_tot;
        if (grad >= sd->gradient_threshold &&
            drop_fraction >= sd->pressure_drop) {
            slip = true;
        }
    }

    /* Store current frame for next comparison */
    memcpy(&sd->prev_frame, frame, sizeof(Frame));

    if (slip) {
        event->timestamp_ms       = frame->timestamp_ms;
        event->frame_id           = frame->frame_id;
        event->gradient_magnitude = grad;
        return true;
    }

    return false;
}