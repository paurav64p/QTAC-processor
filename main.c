#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

#include "src/ring_buffer.h"
#include "src/frame_codec.h"
#include "src/frame_source.h"
#include "src/baseline_filter.h"
#include "src/contact_detector.h"
#include "src/slip_detector.h"
#include "src/centroid_tracker.h"
#include "src/vibration_analyzer.h"
#include "src/event_bus.h"
#include "src/renderer.h"

/* Grid dimensions - change to match your CSV */
#define ROWS 8
#define COLS 8

/* Frame delay in ms - 100 Hz playback */
#define FRAME_DELAY_MS 10

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <data.csv>\n", argv[0]);
        return 1;
    }

    /* Open the CSV data source */
    FrameSource source;
    if (!fs_open(&source, argv[1], ROWS, COLS)) {
        fprintf(stderr, "Error: could not open %s\n", argv[1]);
        return 1;
    }

    /* Initialize all pipeline stages */
    RingBuffer       rb;
    BaselineFilter   bf;
    ContactDetector  cd;
    SlipDetector     sd;
    CentroidTracker  ct;
    VibrationAnalyzer va;
    EventBus         eb;

    rb_init(&rb);
    bf_init(&bf, 0.02f);
    cd_init(&cd, 0.05f, 0.02f);
    sd_init(&sd, 0.03f, 0.10f);
    ct_init(&ct);
    va_init(&va, 1000.0f / FRAME_DELAY_MS);
    eb_init(&eb);

    render_init();

    Frame        frame;
    uint32_t     frame_count  = 0;
    int          slip_active  = 0;

    /* Run continuously - rewind CSV at end of file */
    for (;;) {
        if (!fs_next(&source, &frame)) {
            /* End of file - rewind and reset pipeline state */
            fs_close(&source);
            if (!fs_open(&source, argv[1], ROWS, COLS))
                break;
            bf_reset(&bf);
            sd_init(&sd, 0.03f, 0.10f);
            ct_init(&ct);
            va_init(&va, 1000.0f / FRAME_DELAY_MS);
            continue;
        }

        /* Push raw frame into ring buffer */
        rb_push(&rb, &frame);

        /* Pop and process */
        Frame processed;
        if (!rb_pop(&rb, &processed))
            continue;

        /* Stage 1: baseline drift correction */
        bf_apply(&bf, &processed);

        /* Stage 2: contact detection */
        ContactEvent ce;
        if (cd_process(&cd, &processed, &ce)) {
            BusEvent ev;
            memset(&ev, 0, sizeof(ev));
            ev.type         = EVENT_CONTACT;
            ev.timestamp_ms = ce.timestamp_ms;
            ev.frame_id     = ce.frame_id;
            ev.data.contact = ce;
            eb_push(&eb, &ev);
        }

        /* Stage 3: slip detection */
        SlipEvent se;
        slip_active = 0;
        if (sd_process(&sd, &processed, &se)) {
            slip_active = 1;
            BusEvent ev;
            memset(&ev, 0, sizeof(ev));
            ev.type      = EVENT_SLIP;
            ev.timestamp_ms = se.timestamp_ms;
            ev.frame_id  = se.frame_id;
            ev.data.slip = se;
            eb_push(&eb, &ev);
        }

        /* Stage 4: centroid tracking */
        Centroid centroid;
        int has_centroid = ct_update(&ct, &processed, &centroid);

        /* Stage 5: vibration analysis */
        va_update(&va, &processed);
        float peak_hz = va_peak_freq(&va, ROWS, COLS);

        /* Render */
        render_frame(&processed,
                     &centroid,
                     has_centroid,
                     slip_active,
                     peak_hz,
                     frame_count++);

        sleep_ms(FRAME_DELAY_MS);
    }

    render_cleanup();
    fs_close(&source);
    return 0;
}