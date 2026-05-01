CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -O2 -Isrc
SRCS    = src/ring_buffer.c \
          src/frame_codec.c \
          src/frame_source.c \
          src/baseline_filter.c \
          src/contact_detector.c \
          src/slip_detector.c \
          src/centroid_tracker.c \
          src/vibration_analyzer.c \
          src/event_bus.c \
          src/renderer.c

OBJS    = $(SRCS:.c=.o)

# Main executable
all: qtac_processor

qtac_processor: $(OBJS) main.o
	$(CC) $(CFLAGS) -o $@ $^

# Test executables
test_ring_buffer: src/ring_buffer.o test/test_ring_buffer.o
	$(CC) $(CFLAGS) -o $@ $^

test_frame_codec: src/ring_buffer.o src/frame_codec.o test/test_frame_codec.o
	$(CC) $(CFLAGS) -o $@ $^

test_baseline_filter: src/ring_buffer.o src/baseline_filter.o test/test_baseline_filter.o
	$(CC) $(CFLAGS) -o $@ $^

test_contact_detector: src/ring_buffer.o src/contact_detector.o test/test_contact_detector.o
	$(CC) $(CFLAGS) -o $@ $^

test_slip_detector: src/ring_buffer.o src/slip_detector.o test/test_slip_detector.o
	$(CC) $(CFLAGS) -o $@ $^

# Run all tests
test: test_ring_buffer test_frame_codec test_baseline_filter test_contact_detector test_slip_detector
	./test_ring_buffer
	./test_frame_codec
	./test_baseline_filter
	./test_contact_detector
	./test_slip_detector

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) main.o \
	      qtac_processor \
	      test_ring_buffer test_frame_codec \
	      test_baseline_filter test_contact_detector \
	      test_slip_detector