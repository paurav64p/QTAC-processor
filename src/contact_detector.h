#ifndef CONTACT_DETECTOR_H
#define CONTACT_DETECTOR_H

#include "ring_buffer.h"
#include <stdbool.h>

/* Contact state machine states */
typedef enum {
    CONTACT_NONE = 0,   /* No contact detected       */
    CONTACT_ACTIVE      /* Object is touching sensor  */
} ContactState;

/* Event emitted when contact state changes */
typedef struct {
    ContactState state;         /* New state after transition   */
    uint32_t     timestamp_ms;  /* When the transition happened */
    uint32_t     frame_id;      /* Which frame triggered it     */
    float        peak_pressure; /* Max cell value in the frame  */
} ContactEvent;

typedef struct {
    ContactState current_state;
    float        on_threshold;   /* Pressure to trigger CONTACT_ACTIVE */
    float        off_threshold;  /* Pressure to return to CONTACT_NONE  */
} ContactDetector;

/* Initialize with hysteresis thresholds
 * on_threshold  > off_threshold to prevent rapid toggling
 * Typical: on=0.05, off=0.02 */
void cd_init(ContactDetector *cd, float on_threshold, float off_threshold);

/* Process one frame
 * Returns true and fills event if state changed, false otherwise */
bool cd_process(ContactDetector *cd, const Frame *frame, ContactEvent *event);

#endif /* CONTACT_DETECTOR_H */