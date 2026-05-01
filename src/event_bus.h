#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include "contact_detector.h"
#include "slip_detector.h"
#include "centroid_tracker.h"
#include "vibration_analyzer.h"

/* All possible event types */
typedef enum {
    EVENT_NONE = 0,
    EVENT_CONTACT,      /* Contact state changed        */
    EVENT_SLIP,         /* Slip detected                */
    EVENT_FRAME         /* New frame processed          */
} EventType;

/* One event on the bus */
typedef struct {
    EventType type;
    uint32_t  timestamp_ms;
    uint32_t  frame_id;

    union {
        ContactEvent contact;
        SlipEvent    slip;
    } data;

    /* State snapshot attached to every event */
    Centroid current_centroid;
    float    centroid_displacement;
    float    peak_vibration_hz;
    int      contact_active;
} BusEvent;

/* Capacity of the event queue */
#define EVENT_BUS_CAPACITY 64

typedef struct {
    BusEvent slots[EVENT_BUS_CAPACITY];
    uint32_t head;
    uint32_t tail;
} EventBus;

/* Initialize the event bus */
void eb_init(EventBus *eb);

/* Push an event - returns false if bus is full */
int eb_push(EventBus *eb, const BusEvent *event);

/* Pop an event - returns false if bus is empty */
int eb_pop(EventBus *eb, BusEvent *out);

/* Returns 1 if empty */
int eb_is_empty(const EventBus *eb);

#endif /* EVENT_BUS_H */