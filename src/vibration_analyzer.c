#include "vibration_analyzer.h"
#include <string.h>
#include <math.h>

void va_init(VibrationAnalyzer *va, float sample_rate_hz)
{
    memset(va->history, 0, sizeof(va->history));
    va->index          = 0;
    va->count          = 0;
    va->sample_rate_hz = sample_rate_hz;
}

void va_update(VibrationAnalyzer *va, const Frame *frame)
{
    uint32_t num_cells = (uint32_t)frame->rows * frame->cols;

    for (uint32_t i = 0; i < num_cells; i++)
        va->history[i][va->index] = frame->cells[i];

    va->index = (va->index + 1) % VIBRATION_WINDOW;
    if (va->count < VIBRATION_WINDOW)
        va->count++;
}

/* Count zero crossings in a cell's time series
 * Uses mean as the reference level to handle DC offset */
static int zero_crossings(const float *series, uint32_t len, float mean)
{
    int crossings = 0;
    for (uint32_t i = 1; i < len; i++) {
        float prev = series[i - 1] - mean;
        float curr = series[i]     - mean;
        /* Sign change indicates a crossing */
        if ((prev >= 0.0f && curr < 0.0f) ||
            (prev <  0.0f && curr >= 0.0f))
            crossings++;
    }
    return crossings;
}

void va_analyze(const VibrationAnalyzer *va,
                uint8_t rows, uint8_t cols,
                VibrationResult *out)
{
    uint32_t num_cells = (uint32_t)rows * cols;
    uint32_t n         = va->count < VIBRATION_WINDOW
                         ? va->count : VIBRATION_WINDOW;

    if (n < 4) {
        for (uint32_t i = 0; i < num_cells; i++) {
            out[i].freq_hz = 0.0f;
            out[i].active  = 0;
        }
        return;
    }

    for (uint32_t i = 0; i < num_cells; i++) {
        /* Reconstruct chronological order from circular buffer */
        float series[VIBRATION_WINDOW];
        for (uint32_t j = 0; j < n; j++) {
            uint32_t idx = (va->index + VIBRATION_WINDOW - n + j)
                           % VIBRATION_WINDOW;
            series[j] = va->history[i][idx];
        }

        /* Compute mean */
        float sum = 0.0f;
        for (uint32_t j = 0; j < n; j++)
            sum += series[j];
        float mean = sum / (float)n;

        /* Compute variance to check if signal is active */
        float var = 0.0f;
        for (uint32_t j = 0; j < n; j++) {
            float d = series[j] - mean;
            var += d * d;
        }
        var /= (float)n;

        int zc = zero_crossings(series, n, mean);

        /* Frequency = zero crossings / 2 / time window */
        float duration_s = (float)n / va->sample_rate_hz;
        out[i].freq_hz   = (float)zc / 2.0f / duration_s;
        out[i].active    = (var > 1e-6f) ? 1 : 0;
    }
}

float va_peak_freq(const VibrationAnalyzer *va,
                   uint8_t rows, uint8_t cols)
{
    uint32_t num_cells = (uint32_t)rows * cols;
    VibrationResult results[FRAME_MAX_CELLS];
    va_analyze(va, rows, cols, results);

    float peak = 0.0f;
    for (uint32_t i = 0; i < num_cells; i++) {
        if (results[i].active && results[i].freq_hz > peak)
            peak = results[i].freq_hz;
    }
    return peak;
}