#include "event_bus.h"
#include <string.h>

void eb_init(EventBus *eb)
{
    memset(eb->slots, 0, sizeof(eb->slots));
    eb->head = 0;
    eb->tail = 0;
}

int eb_is_empty(const EventBus *eb)
{
    return eb->head == eb->tail;
}

int eb_push(EventBus *eb, const BusEvent *event)
{
    uint32_t next = (eb->head + 1) % EVENT_BUS_CAPACITY;
    if (next == eb->tail)
        return 0; /* Full */

    eb->slots[eb->head] = *event;
    eb->head = next;
    return 1;
}

int eb_pop(EventBus *eb, BusEvent *out)
{
    if (eb_is_empty(eb))
        return 0;

    *out     = eb->slots[eb->tail];
    eb->tail = (eb->tail + 1) % EVENT_BUS_CAPACITY;
    return 1;
}