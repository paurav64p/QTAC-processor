#include "baseline_filter.h"
#include <string.h>

void bf_init(BaselineFilter *bf, float alpha)
{
    memset(bf->baseline, 0, sizeof(bf->baseline));
    bf->alpha       = alpha;
    bf->initialized = 0;
}

void bf_apply(BaselineFilter *bf, Frame *frame)
{
    uint32_t num_cells = (uint32_t)frame->rows * frame->cols;

    /* On the very first frame seed the baseline with raw values */
    if (!bf->initialized) {
        for (uint32_t i = 0; i < num_cells; i++)
            bf->baseline[i] = frame->cells[i];
        bf->initialized = 1;
    }

    for (uint32_t i = 0; i < num_cells; i++) {
        /* Update baseline with exponential moving average */
        bf->baseline[i] = bf->alpha * frame->cells[i]
                        + (1.0f - bf->alpha) * bf->baseline[i];

        /* Subtract baseline to get net contact pressure */
        float corrected = frame->cells[i] - bf->baseline[i];

        /* Clamp negative values to zero */
        frame->cells[i] = corrected > 0.0f ? corrected : 0.0f;
    }
}

void bf_reset(BaselineFilter *bf)
{
    memset(bf->baseline, 0, sizeof(bf->baseline));
    bf->initialized = 0;
}