#include "contact_detector.h"
#include <string.h>

void cd_init(ContactDetector *cd, float on_threshold, float off_threshold)
{
    cd->current_state = CONTACT_NONE;
    cd->on_threshold  = on_threshold;
    cd->off_threshold = off_threshold;
}

bool cd_process(ContactDetector *cd, const Frame *frame, ContactEvent *event)
{
    uint32_t num_cells   = (uint32_t)frame->rows * frame->cols;
    float    peak        = 0.0f;

    /* Find the highest pressure cell in this frame */
    for (uint32_t i = 0; i < num_cells; i++) {
        if (frame->cells[i] > peak)
            peak = frame->cells[i];
    }

    ContactState new_state = cd->current_state;

    if (cd->current_state == CONTACT_NONE) {
        /* Waiting for contact - check on threshold */
        if (peak >= cd->on_threshold)
            new_state = CONTACT_ACTIVE;
    } else {
        /* Contact active - check off threshold */
        if (peak < cd->off_threshold)
            new_state = CONTACT_NONE;
    }

    /* No state change */
    if (new_state == cd->current_state)
        return false;

    /* State changed - fill event and update */
    cd->current_state  = new_state;
    event->state        = new_state;
    event->timestamp_ms = frame->timestamp_ms;
    event->frame_id     = frame->frame_id;
    event->peak_pressure = peak;

    return true;
}