# QTAC Tactile Stream Processor

A real-time tactile sensor data processing pipeline written in C.

---

## What this is

This project processes a continuous stream of pressure frames from a capacitive tactile sensor grid and analyses them in real time. It was built with H4 Lab's QTAC100 sensor in mind, focusing on the signal processing layer that sits directly above the hardware driver.

The input is a CSV file where each row is one frame of pressure readings from an 8x8 sensor grid. The output is a live colour-coded terminal display showing contact pressure, centroid position, vibration frequency, and slip detection.

---

## What it does

**Baseline drift correction** — Capacitive sensors drift over time even with no contact. An exponential moving average filter tracks and removes this drift per cell so only real contact pressure remains.

**Contact detection** — A hysteresis threshold state machine detects when an object makes or breaks contact with the sensor surface.

**Slip detection** — Consecutive frames are compared for spatial gradient shifts combined with pressure drops. If both exceed their thresholds, a slip warning is raised immediately.

**Centroid tracking** — The weighted centre of mass of the contact region is computed every frame and tracked over time. Shown as a live marker on the terminal grid.

**Vibration analysis** — A zero-crossing counter runs over a sliding window of each cell's time series to estimate vibration frequency. No FFT, no external library, integer math only.

---

## Project structure

```
qtac_processor/
├── src/
│   ├── ring_buffer.c / .h        Lock-free frame queue (C11 atomics)
│   ├── frame_codec.c / .h        Frame validation and unpacking
│   ├── frame_source.c / .h       CSV file reader
│   ├── baseline_filter.c / .h    Exponential moving average drift correction
│   ├── contact_detector.c / .h   Hysteresis contact state machine
│   ├── slip_detector.c / .h      Gradient-based slip detection
│   ├── centroid_tracker.c / .h   Weighted centre of mass tracker
│   ├── vibration_analyzer.c / .h Zero-crossing frequency estimator
│   ├── event_bus.c / .h          Event aggregator
│   └── renderer.c / .h           ANSI terminal heatmap renderer
├── test/
│   ├── test_ring_buffer.c
│   ├── test_frame_codec.c
│   ├── test_baseline_filter.c
│   ├── test_contact_detector.c
│   └── test_slip_detector.c
├── data/
│   └── sample.csv
├── main.c
└── Makefile
```

---

## Dependencies

- GCC with C11 support (GCC 9 or later recommended)
- GNU Make
- No external libraries beyond the C standard library

---

## Build

```bash
make qtac_processor
```

On Windows with MSYS2 UCRT64:

```bash
mingw32-make qtac_processor
```

---

## Run

```bash
./qtac_processor data/sample.csv
```

The program loops the CSV continuously. Press `Ctrl+C` to stop.

---

## Run tests

```bash
make test
```

On Windows:

```bash
mingw32-make test
```

27 tests across 5 modules, all passing.

---

## CSV format

Each row is one pressure frame. Values are comma separated floats in the range 0.0 to 1.0. The number of values per row must equal rows x cols as defined in main.c (default 8x8 = 64 values per row).

```
0.0012,0.0034,0.0021,...
0.0015,0.0890,0.3421,...
```

Comment lines starting with `#` and blank lines are skipped.

---

## Design notes

The ring buffer uses C11 atomics with acquire/release memory ordering for lock-free single-producer single-consumer operation. There is no dynamic memory allocation in the processing hot path. The renderer uses ANSI cursor repositioning instead of clearing the screen on each frame to avoid flicker.

The pipeline is hardware agnostic. It processes any rectangular grid of normalised pressure values regardless of the physical sensor that produced them.