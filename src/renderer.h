#ifndef RENDERER_H
#define RENDERER_H

#include "ring_buffer.h"
#include "event_bus.h"

/* Initialize the renderer - clears terminal and hides cursor */
void render_init(void);

/* Draw one complete frame to the terminal
 * frame:         the pressure grid to render
 * centroid:      current contact centre of mass
 * has_centroid:  1 if centroid is valid this frame
 * slip_active:   1 if a slip event was detected
 * peak_freq_hz:  peak vibration frequency across all cells
 * frame_count:   total frames processed so far */
void render_frame(const Frame    *frame,
                  const Centroid *centroid,
                  int             has_centroid,
                  int             slip_active,
                  float           peak_freq_hz,
                  uint32_t        frame_count);

/* Restore terminal on exit */
void render_cleanup(void);

#endif /* RENDERER_H */