#ifndef VIBRATION_ANALYZER_H
#define VIBRATION_ANALYZER_H

#include "ring_buffer.h"

/* Number of frames kept for zero crossing analysis */
#define VIBRATION_WINDOW 32

typedef struct {
    float    history[FRAME_MAX_CELLS][VIBRATION_WINDOW]; /* Per cell history */
    uint32_t index;         /* Next write position in window                  */
    uint32_t count;         /* How many frames have been recorded             */
    float    sample_rate_hz;/* Frame rate of the sensor (e.g. 100.0)         */
} VibrationAnalyzer;

/* Result per cell after analysis */
typedef struct {
    float freq_hz;          /* Estimated vibration frequency in Hz           */
    int   active;           /* 1 if meaningful vibration detected            */
} VibrationResult;

/* Initialize with the sensor frame rate in Hz */
void va_init(VibrationAnalyzer *va, float sample_rate_hz);

/* Feed one frame into the analyzer */
void va_update(VibrationAnalyzer *va, const Frame *frame);

/* Compute vibration frequency estimate for each cell
 * out must point to an array of at least rows*cols VibrationResult */
void va_analyze(const VibrationAnalyzer *va,
                uint8_t rows, uint8_t cols,
                VibrationResult *out);

/* Return the peak vibration frequency across all cells */
float va_peak_freq(const VibrationAnalyzer *va,
                   uint8_t rows, uint8_t cols);

#endif /* VIBRATION_ANALYZER_H */