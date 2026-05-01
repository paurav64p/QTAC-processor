#include <stdio.h>
#include <assert.h>
#include "../src/frame_codec.h"

static void test_valid_frame(void)
{
    float cells[16];
    for (int i = 0; i < 16; i++)
        cells[i] = (float)i / 16.0f;

    Frame out;
    CodecResult r = frame_parse(&out, 1, 100, 0, 4, 4, cells);
    assert(r == CODEC_OK);
    assert(out.frame_id     == 1);
    assert(out.timestamp_ms == 100);
    assert(out.rows         == 4);
    assert(out.cols         == 4);
    assert(out.cells[0]     == cells[0]);
    assert(out.cells[15]    == cells[15]);
    printf("  [PASS] valid frame parses correctly\n");
}

static void test_null_pointer(void)
{
    float cells[16] = {0};
    Frame out;
    assert(frame_parse(NULL,  1, 100, 0, 4, 4, cells) == CODEC_ERR_NULL);
    assert(frame_parse(&out,  1, 100, 0, 4, 4, NULL)  == CODEC_ERR_NULL);
    printf("  [PASS] NULL pointers rejected\n");
}

static void test_zero_dimensions(void)
{
    float cells[16] = {0};
    Frame out;
    assert(frame_parse(&out, 1, 100, 0, 0, 4, cells) == CODEC_ERR_BAD_DIMS);
    assert(frame_parse(&out, 1, 100, 0, 4, 0, cells) == CODEC_ERR_BAD_DIMS);
    printf("  [PASS] zero dimensions rejected\n");
}

static void test_too_large(void)
{
    float cells[FRAME_MAX_CELLS + 1];
    for (int i = 0; i <= (int)FRAME_MAX_CELLS; i++)
        cells[i] = 0.0f;
    Frame out;
    /* 9x9 = 81 > FRAME_MAX_CELLS (64) */
    assert(frame_parse(&out, 1, 100, 0, 9, 9, cells) == CODEC_ERR_TOO_LARGE);
    printf("  [PASS] oversized frame rejected\n");
}

static void test_bad_values(void)
{
    float cells[16] = {0};
    cells[5] = 1.5f;  /* Out of range */
    Frame out;
    assert(frame_parse(&out, 1, 100, 0, 4, 4, cells) == CODEC_ERR_BAD_VALUES);

    cells[5] = -0.1f; /* Negative */
    assert(frame_parse(&out, 1, 100, 0, 4, 4, cells) == CODEC_ERR_BAD_VALUES);
    printf("  [PASS] out of range cell values rejected\n");
}

static void test_bad_timestamp(void)
{
    float cells[16] = {0};
    Frame out;
    /* timestamp going backwards */
    assert(frame_parse(&out, 2, 50, 100, 4, 4, cells) == CODEC_ERR_BAD_TIMESTAMP);
    /* equal timestamp also rejected when prev > 0 */
    assert(frame_parse(&out, 2, 100, 100, 4, 4, cells) == CODEC_ERR_BAD_TIMESTAMP);
    printf("  [PASS] backwards timestamp rejected\n");
}

static void test_codec_result_str(void)
{
    assert(codec_result_str(CODEC_OK)            != 0);
    assert(codec_result_str(CODEC_ERR_NULL)      != 0);
    assert(codec_result_str(CODEC_ERR_BAD_DIMS)  != 0);
    printf("  [PASS] codec_result_str returns strings\n");
}

int main(void)
{
    printf("frame_codec tests\n");
    test_valid_frame();
    test_null_pointer();
    test_zero_dimensions();
    test_too_large();
    test_bad_values();
    test_bad_timestamp();
    test_codec_result_str();
    printf("All frame_codec tests passed.\n");
    return 0;
}