#include <stdio.h>
#include <assert.h>
#include "../src/contact_detector.h"

static Frame make_frame_peak(uint8_t rows, uint8_t cols, float peak)
{
    Frame f = {0};
    f.rows = rows;
    f.cols = cols;
    f.timestamp_ms = 10;
    f.frame_id     = 1;
    /* Put peak in the centre cell */
    uint32_t centre = ((uint32_t)rows / 2) * cols + cols / 2;
    f.cells[centre] = peak;
    return f;
}

static void test_init(void)
{
    ContactDetector cd;
    cd_init(&cd, 0.05f, 0.02f);
    assert(cd.current_state  == CONTACT_NONE);
    assert(cd.on_threshold   == 0.05f);
    assert(cd.off_threshold  == 0.02f);
    printf("  [PASS] init\n");
}

static void test_no_contact_below_threshold(void)
{
    ContactDetector cd;
    cd_init(&cd, 0.05f, 0.02f);

    Frame f = make_frame_peak(4, 4, 0.03f);
    ContactEvent ev;
    assert(!cd_process(&cd, &f, &ev));
    assert(cd.current_state == CONTACT_NONE);
    printf("  [PASS] below threshold stays CONTACT_NONE\n");
}

static void test_contact_onset(void)
{
    ContactDetector cd;
    cd_init(&cd, 0.05f, 0.02f);

    Frame f = make_frame_peak(4, 4, 0.10f);
    ContactEvent ev;
    assert(cd_process(&cd, &f, &ev));
    assert(cd.current_state == CONTACT_ACTIVE);
    assert(ev.state         == CONTACT_ACTIVE);
    assert(ev.peak_pressure >= 0.10f);
    printf("  [PASS] above threshold triggers CONTACT_ACTIVE\n");
}

static void test_contact_offset(void)
{
    ContactDetector cd;
    cd_init(&cd, 0.05f, 0.02f);

    /* First trigger contact */
    Frame high = make_frame_peak(4, 4, 0.10f);
    ContactEvent ev;
    cd_process(&cd, &high, &ev);
    assert(cd.current_state == CONTACT_ACTIVE);

    /* Now drop below off threshold */
    Frame low = make_frame_peak(4, 4, 0.01f);
    assert(cd_process(&cd, &low, &ev));
    assert(cd.current_state == CONTACT_NONE);
    assert(ev.state         == CONTACT_NONE);
    printf("  [PASS] below off_threshold triggers CONTACT_NONE\n");
}

static void test_hysteresis(void)
{
    ContactDetector cd;
    cd_init(&cd, 0.05f, 0.02f);

    /* Trigger contact */
    Frame high = make_frame_peak(4, 4, 0.10f);
    ContactEvent ev;
    cd_process(&cd, &high, &ev);

    /* Drop to between off and on thresholds - should stay active */
    Frame mid = make_frame_peak(4, 4, 0.03f);
    assert(!cd_process(&cd, &mid, &ev));
    assert(cd.current_state == CONTACT_ACTIVE);
    printf("  [PASS] hysteresis keeps CONTACT_ACTIVE between thresholds\n");
}

int main(void)
{
    printf("contact_detector tests\n");
    test_init();
    test_no_contact_below_threshold();
    test_contact_onset();
    test_contact_offset();
    test_hysteresis();
    printf("All contact_detector tests passed.\n");
    return 0;
}