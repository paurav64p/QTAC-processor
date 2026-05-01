#include <stdio.h>
#include <assert.h>
#include "../src/slip_detector.h"

static Frame make_uniform_frame(uint8_t rows, uint8_t cols,
                                float val, uint32_t ts, uint32_t id)
{
    Frame f = {0};
    f.rows         = rows;
    f.cols         = cols;
    f.timestamp_ms = ts;
    f.frame_id     = id;
    uint32_t n = (uint32_t)rows * cols;
    for (uint32_t i = 0; i < n; i++)
        f.cells[i] = val;
    return f;
}

static void test_init(void)
{
    SlipDetector sd;
    sd_init(&sd, 0.03f, 0.10f);
    assert(sd.initialized        == 0);
    assert(sd.gradient_threshold == 0.03f);
    assert(sd.pressure_drop      == 0.10f);
    printf("  [PASS] init\n");
}

static void test_first_frame_no_slip(void)
{
    SlipDetector sd;
    sd_init(&sd, 0.03f, 0.10f);

    Frame f = make_uniform_frame(4, 4, 0.5f, 10, 0);
    SlipEvent ev;
    assert(!sd_process(&sd, &f, &ev));
    printf("  [PASS] first frame never triggers slip\n");
}

static void test_stable_grasp_no_slip(void)
{
    SlipDetector sd;
    sd_init(&sd, 0.03f, 0.10f);

    SlipEvent ev;
    /* Two identical frames - no gradient, no pressure drop */
    Frame f1 = make_uniform_frame(4, 4, 0.5f, 10, 0);
    Frame f2 = make_uniform_frame(4, 4, 0.5f, 20, 1);
    sd_process(&sd, &f1, &ev);
    assert(!sd_process(&sd, &f2, &ev));
    printf("  [PASS] stable grasp does not trigger slip\n");
}

static void test_slip_detected(void)
{
    SlipDetector sd;
    sd_init(&sd, 0.03f, 0.10f);

    SlipEvent ev;

    /* Frame 1 - high uniform pressure */
    Frame f1 = make_uniform_frame(4, 4, 0.8f, 10, 0);
    sd_process(&sd, &f1, &ev);

    /* Frame 2 - shifted pattern with pressure drop */
    Frame f2 = {0};
    f2.rows         = 4;
    f2.cols         = 4;
    f2.timestamp_ms = 20;
    f2.frame_id     = 1;
    /* Put pressure only in first few cells - big shift from uniform */
    f2.cells[0] = 0.5f;
    f2.cells[1] = 0.5f;
    /* rest are 0 - large gradient + large pressure drop */

    assert(sd_process(&sd, &f2, &ev));
    assert(ev.frame_id == 1);
    assert(ev.gradient_magnitude > 0.0f);
    printf("  [PASS] large gradient with pressure drop triggers slip\n");
}

static void test_no_slip_without_pressure_drop(void)
{
    SlipDetector sd;
    sd_init(&sd, 0.03f, 0.10f);

    SlipEvent ev;

    /* Frame 1 */
    Frame f1 = make_uniform_frame(4, 4, 0.5f, 10, 0);
    sd_process(&sd, &f1, &ev);

    /* Frame 2 - same total pressure, just redistributed slightly */
    Frame f2 = make_uniform_frame(4, 4, 0.5f, 20, 1);
    f2.cells[0] = 0.55f;
    f2.cells[15] = 0.45f;

    /* Small redistribution, no significant drop - no slip */
    assert(!sd_process(&sd, &f2, &ev));
    printf("  [PASS] pressure redistribution without drop does not trigger slip\n");
}

int main(void)
{
    printf("slip_detector tests\n");
    test_init();
    test_first_frame_no_slip();
    test_stable_grasp_no_slip();
    test_slip_detected();
    test_no_slip_without_pressure_drop();
    printf("All slip_detector tests passed.\n");
    return 0;
}