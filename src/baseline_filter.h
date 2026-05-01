#ifndef BASELINE_FILTER_H
#define BASELINE_FILTER_H

#include "ring_buffer.h"

/* Exponential moving average filter applied per cell
 * Tracks the resting baseline and subtracts it from each frame
 * so that only actual contact pressure remains */
typedef struct {
    float baseline[FRAME_MAX_CELLS]; /* Current baseline per cell   */
    float alpha;                     /* Smoothing factor 0.0 to 1.0
                                        Lower = slower drift correction
                                        Typical: 0.01 to 0.05        */
    int   initialized;               /* 0 until first frame seen     */
} BaselineFilter;

/* Initialize the filter with a smoothing factor alpha */
void bf_init(BaselineFilter *bf, float alpha);

/* Apply the filter to a frame in place
 * Subtracts the current baseline from each cell
 * Negative results are clamped to 0.0 */
void bf_apply(BaselineFilter *bf, Frame *frame);

/* Reset the baseline - call if sensor is lifted and replaced */
void bf_reset(BaselineFilter *bf);

#endif /* BASELINE_FILTER_H */