#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/ring_buffer.h"

static void test_init(void)
{
    RingBuffer rb;
    rb_init(&rb);
    assert(rb_is_empty(&rb));
    assert(!rb_is_full(&rb));
    printf("  [PASS] init\n");
}

static void test_push_pop_single(void)
{
    RingBuffer rb;
    rb_init(&rb);

    Frame in = {0};
    in.frame_id      = 1;
    in.timestamp_ms  = 100;
    in.rows          = 4;
    in.cols          = 4;
    in.cells[0]      = 0.5f;
    in.cells[15]     = 1.0f;

    assert(rb_push(&rb, &in));
    assert(!rb_is_empty(&rb));

    Frame out = {0};
    assert(rb_pop(&rb, &out));
    assert(rb_is_empty(&rb));

    assert(out.frame_id     == 1);
    assert(out.timestamp_ms == 100);
    assert(out.rows         == 4);
    assert(out.cols         == 4);
    assert(out.cells[0]     == 0.5f);
    assert(out.cells[15]    == 1.0f);

    printf("  [PASS] push and pop single frame\n");
}

static void test_full_buffer(void)
{
    RingBuffer rb;
    rb_init(&rb);

    Frame f = {0};
    for (int i = 0; i < RING_BUFFER_CAPACITY; i++) {
        f.frame_id = (uint32_t)i;
        assert(rb_push(&rb, &f));
    }

    assert(rb_is_full(&rb));

    /* One more push must fail */
    f.frame_id = 999;
    assert(!rb_push(&rb, &f));

    printf("  [PASS] buffer full rejects extra push\n");
}

static void test_fifo_order(void)
{
    RingBuffer rb;
    rb_init(&rb);

    Frame f = {0};
    for (int i = 0; i < 8; i++) {
        f.frame_id = (uint32_t)i;
        rb_push(&rb, &f);
    }

    Frame out = {0};
    for (int i = 0; i < 8; i++) {
        assert(rb_pop(&rb, &out));
        assert(out.frame_id == (uint32_t)i);
    }

    printf("  [PASS] FIFO order preserved\n");
}

static void test_pop_empty(void)
{
    RingBuffer rb;
    rb_init(&rb);

    Frame out = {0};
    assert(!rb_pop(&rb, &out));

    printf("  [PASS] pop on empty returns false\n");
}

int main(void)
{
    printf("ring_buffer tests\n");
    test_init();
    test_push_pop_single();
    test_full_buffer();
    test_fifo_order();
    test_pop_empty();
    printf("All ring_buffer tests passed.\n");
    return 0;
}
