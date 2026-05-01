#include "renderer.h"
#include <stdio.h>
#include <math.h>

/* ANSI escape codes */
#define ANSI_RESET      "\x1b[0m"
#define ANSI_BOLD       "\x1b[1m"
#define ANSI_CLEAR      "\x1b[2J"
#define ANSI_HOME       "\x1b[H"
#define ANSI_HIDE_CURSOR "\x1b[?25l"
#define ANSI_SHOW_CURSOR "\x1b[?25h"

/* Pressure levels mapped to background colours
 * Uses 256-colour ANSI: \x1b[48;5;<n>m
 * 232-255 is a greyscale ramp, 196+ reds for high pressure */
static const char *pressure_colour(float val)
{
    if (val < 0.05f) return "\x1b[48;5;232m";  /* near black - no contact  */
    if (val < 0.15f) return "\x1b[48;5;17m";   /* dark blue                */
    if (val < 0.25f) return "\x1b[48;5;18m";   /* blue                     */
    if (val < 0.35f) return "\x1b[48;5;21m";   /* bright blue              */
    if (val < 0.45f) return "\x1b[48;5;27m";   /* blue-cyan                */
    if (val < 0.55f) return "\x1b[48;5;34m";   /* green                    */
    if (val < 0.65f) return "\x1b[48;5;226m";  /* yellow                   */
    if (val < 0.75f) return "\x1b[48;5;208m";  /* orange                   */
    if (val < 0.90f) return "\x1b[48;5;196m";  /* red                      */
    return               "\x1b[48;5;199m";      /* bright magenta - peak    */
}

void render_init(void)
{
    printf(ANSI_CLEAR ANSI_HOME ANSI_HIDE_CURSOR);
    fflush(stdout);
}

void render_frame(const Frame    *frame,
                  const Centroid *centroid,
                  int             has_centroid,
                  int             slip_active,
                  float           peak_freq_hz,
                  uint32_t        frame_count)
{
    /* Move cursor to top left without clearing - avoids flicker */
    printf(ANSI_HOME);

    /* Title bar */
    printf(ANSI_BOLD "  QTAC Tactile Stream Processor" ANSI_RESET
           "   frame: %5u\n\n", frame_count);

    /* Pressure grid */
    for (uint8_t r = 0; r < frame->rows; r++) {
        printf("  ");
        for (uint8_t c = 0; c < frame->cols; c++) {
            float val = frame->cells[r * frame->cols + c];

            /* Mark centroid cell with a cross */
            int is_centroid = 0;
            if (has_centroid) {
                int cr = (int)roundf(centroid->row);
                int cc = (int)roundf(centroid->col);
                if (cr == (int)r && cc == (int)c)
                    is_centroid = 1;
            }

            if (is_centroid) {
                printf("\x1b[48;5;255m\x1b[30m" " + " ANSI_RESET);
            } else {
                printf("%s   " ANSI_RESET, pressure_colour(val));
            }
        }
        printf("\n");
    }

    printf("\n");

    /* Status line - centroid */
    if (has_centroid) {
        printf("  Centroid   row: %5.2f   col: %5.2f\n",
               centroid->row, centroid->col);
    } else {
        printf("  Centroid   --no contact--              \n");
    }

    /* Vibration */
    printf("  Vibration  peak: %6.1f Hz              \n", peak_freq_hz);

    /* Slip warning */
    if (slip_active) {
        printf("\n  " ANSI_BOLD "\x1b[41m" "  !! SLIP DETECTED !!  " ANSI_RESET "\n");
    } else {
        printf("\n  \x1b[42m" "     grasp stable     " ANSI_RESET "\n");
    }

    /* Colour legend */
    printf("\n  Low ");
    const float levels[] = {0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.85f,0.95f};
    for (int i = 0; i < 10; i++)
        printf("%s   " ANSI_RESET, pressure_colour(levels[i]));
    printf(" High\n");

    fflush(stdout);
}

void render_cleanup(void)
{
    printf(ANSI_SHOW_CURSOR ANSI_RESET "\n");
    fflush(stdout);
}