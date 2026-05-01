#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../src/baseline_filter.h"

static Frame make_frame(uint8_t rows, uint8_t cols, float val)
{
    Frame f = {0};
    f.rows = rows;
    f.cols = cols;
    uint32_t n = (uint32_t)rows * cols;
    for (uint32_t i = 0; i < n; i++)
        f.cells[i] = val;
    return f;
}

static void test_init(void)
{
    BaselineFilter bf;
    bf_init(&bf, 0.02f);
    assert(bf.alpha       == 0.02f);
    assert(bf.initialized == 0);
    printf("  [PASS] init\n");
}

static void test_first_frame_zeroed(void)
{
    BaselineFilter bf;
    bf_init(&bf, 0.02f);

    /* First frame seeds the baseline - output should be near zero */
    Frame f = make_frame(4, 4, 0.5f);
    bf_apply(&bf, &f);

    uint32_t n = 4 * 4;
    for (uint32_t i = 0; i < n; i++)
        assert(f.cells[i] == 0.0f);

    printf("  [PASS] first frame output is zeroed\n");
}

static void test_contact_above_baseline(void)
{
    BaselineFilter bf;
    bf_init(&bf, 0.02f);

    /* Settle baseline with low ambient frames */
    Frame ambient = make_frame(4, 4, 0.05f);
    for (int i = 0; i < 50; i++) {
        Frame tmp = ambient;
        bf_apply(&bf, &tmp);
    }

    /* Now apply a high pressure frame */
    Frame contact = make_frame(4, 4, 0.8f);
    bf_apply(&bf, &contact);

    /* After baseline subtraction values should be clearly positive */
    uint32_t n = 4 * 4;
    for (uint32_t i = 0; i < n; i++)
        assert(contact.cells[i] > 0.0f);

    printf("  [PASS] contact pressure above baseline is positive\n");
}

static void test_clamp_negative(void)
{
    BaselineFilter bf;
    bf_init(&bf, 0.5f);

    /* Seed with high baseline */
    Frame high = make_frame(4, 4, 0.9f);
    bf_apply(&bf, &high);

    /* Apply lower pressure - should clamp to 0 not go negative */
    Frame low = make_frame(4, 4, 0.1f);
    bf_apply(&bf, &low);

    uint32_t n = 4 * 4;
    for (uint32_t i = 0; i < n; i++)
        assert(low.cells[i] >= 0.0f);

    printf("  [PASS] negative values clamped to zero\n");
}

static void test_reset(void)
{
    BaselineFilter bf;
    bf_init(&bf, 0.02f);

    Frame f = make_frame(4, 4, 0.5f);
    bf_apply(&bf, &f);
    assert(bf.initialized == 1);

    bf_reset(&bf);
    assert(bf.initialized == 0);

    printf("  [PASS] reset clears initialized flag\n");
}

int main(void)
{
    printf("baseline_filter tests\n");
    test_init();
    test_first_frame_zeroed();
    test_contact_above_baseline();
    test_clamp_negative();
    test_reset();
    printf("All baseline_filter tests passed.\n");
    return 0;
}