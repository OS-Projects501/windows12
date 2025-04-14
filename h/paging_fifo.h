#pragma once

#include "paging.h"

#define MAX_FIFO_SIZE NFRAMES

void fifo_init();

int fifo_is_empty(); // return 1 if fifo is empty, 0 otherwise
int fifo_is_full(); // return 1 if fifo is full, 0 otherwise

int fifo_pop_victim(); // give frame id that is to be evicted, -1 when fifo is empty
int fifo_push_frame(int frame); // push frame to fifo, -1 on error (fifo is full)
int fifo_remove_frame(int frame); // remove frame from fifo (on frame free), 1 on success

void fifo_print_debug();