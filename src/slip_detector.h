#ifndef SLIP_DETECTOR_H
#define SLIP_DETECTOR_H

#include "ring_buffer.h"
#include <stdbool.h>

/* Slip detection using spatial gradient between consecutive frames
 * A sudden shift in pressure centroid combined with pressure drop
 * indicates the object is beginning to slide */

typedef struct {
    Frame    prev_frame;        /* Last frame seen                      */
    int      initialized;       /* 0 until first frame processed        */
    float    gradient_threshold;/* Minimum gradient magnitude for slip  */
    float    pressure_drop;     /* Minimum pressure drop fraction       */
} SlipDetector;

/* Event emitted when slip is detected */
typedef struct {
    uint32_t timestamp_ms;      /* When slip was detected               */
    uint32_t frame_id;          /* Frame that triggered detection       */
    float    gradient_magnitude;/* How large the spatial shift was      */
} SlipEvent;

/* Initialize with thresholds
 * gradient_threshold: how much the pressure pattern must shift
 * pressure_drop:      fractional drop in total pressure (0.0 to 1.0)
 * Typical: gradient_threshold=0.03, pressure_drop=0.10 */
void sd_init(SlipDetector *sd,
             float gradient_threshold,
             float pressure_drop);

/* Process one frame
 * Returns true and fills event if slip detected, false otherwise */
bool sd_process(SlipDetector *sd,
                const Frame  *frame,
                SlipEvent    *event);

#endif /* SLIP_DETECTOR_H */