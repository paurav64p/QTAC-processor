#include "ring_buffer.h"
#include <string.h>

void rb_init(RingBuffer *rb)
{
    atomic_store(&rb->head, 0);
    atomic_store(&rb->tail, 0);
}

bool rb_is_empty(const RingBuffer *rb)
{
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    return head == tail;
}

bool rb_is_full(const RingBuffer *rb)
{
    uint32_t head = atomic_load(&rb->head);
    uint32_t tail = atomic_load(&rb->tail);
    return (head - tail) == RING_BUFFER_CAPACITY;
}

bool rb_push(RingBuffer *rb, const Frame *frame)
{
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

    /* Buffer full — drop the frame */
    if ((head - tail) == RING_BUFFER_CAPACITY) {
        return false;
    }

    /* Copy frame into the next available slot */
    uint32_t slot = head % RING_BUFFER_CAPACITY;
    memcpy(&rb->slots[slot], frame, sizeof(Frame));

    /* Publish the write — any consumer will now see this frame */
    atomic_store_explicit(&rb->head, head + 1, memory_order_release);

    return true;
}

bool rb_pop(RingBuffer *rb, Frame *out)
{
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_acquire);

    /* Buffer empty — nothing to read */
    if (tail == head) {
        return false;
    }

    /* Copy frame out of the slot */
    uint32_t slot = tail % RING_BUFFER_CAPACITY;
    memcpy(out, &rb->slots[slot], sizeof(Frame));

    /* Advance tail so producer can reuse this slot */
    atomic_store_explicit(&rb->tail, tail + 1, memory_order_release);

    return true;
}
