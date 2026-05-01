#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>

/* Maximum number of cells in one pressure frame (rows x cols) */
#define FRAME_MAX_CELLS 64

/* Number of frames the ring buffer can hold at once */
#define RING_BUFFER_CAPACITY 32

/* One snapshot from the tactile sensor grid */
typedef struct {
    uint32_t frame_id;                  /* Sequential frame number          */
    uint32_t timestamp_ms;              /* Time in milliseconds              */
    uint8_t  rows;                      /* Number of rows in the grid        */
    uint8_t  cols;                      /* Number of cols in the grid        */
    float    cells[FRAME_MAX_CELLS];    /* Pressure values, row-major order  */
} Frame;

/* Lock-free single-producer single-consumer ring buffer */
typedef struct {
    Frame            slots[RING_BUFFER_CAPACITY];
    atomic_uint      head;   /* Writer advances this */
    atomic_uint      tail;   /* Reader advances this */
} RingBuffer;

/* Initialize the ring buffer - call once before use */
void rb_init(RingBuffer *rb);

/* Push a frame into the buffer (producer side)
 * Returns true on success, false if buffer is full */
bool rb_push(RingBuffer *rb, const Frame *frame);

/* Pop a frame from the buffer (consumer side)
 * Returns true on success, false if buffer is empty */
bool rb_pop(RingBuffer *rb, Frame *out);

/* Returns true if the buffer is empty */
bool rb_is_empty(const RingBuffer *rb);

/* Returns true if the buffer is full */
bool rb_is_full(const RingBuffer *rb);

#endif /* RING_BUFFER_H */