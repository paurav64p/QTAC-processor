#ifndef CENTROID_TRACKER_H
#define CENTROID_TRACKER_H

#include "ring_buffer.h"
#include <stdbool.h>

/* Sliding window size for centroid history */
#define CENTROID_HISTORY 16

typedef struct {
    float row;  /* Row coordinate of centroid   */
    float col;  /* Column coordinate of centroid */
} Centroid;

typedef struct {
    Centroid history[CENTROID_HISTORY]; /* Recent centroid positions    */
    uint32_t count;                     /* How many entries are valid   */
    uint32_t index;                     /* Next write position          */
} CentroidTracker;

/* Initialize the tracker */
void ct_init(CentroidTracker *ct);

/* Compute centroid for one frame and store it in the history
 * Returns false if total pressure is zero (nothing touching) */
bool ct_update(CentroidTracker *ct, const Frame *frame, Centroid *out);

/* Return the most recent centroid without updating */
bool ct_latest(const CentroidTracker *ct, Centroid *out);

/* Compute how far the centroid has moved over the last N frames
 * Useful for detecting gradual drift during a grasp */
float ct_displacement(const CentroidTracker *ct, uint32_t n_frames);

#endif /* CENTROID_TRACKER_H */