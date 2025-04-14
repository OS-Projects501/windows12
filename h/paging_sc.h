#pragma once

#include "paging.h"

#define MAX_SC_SIZE NFRAMES

void sc_init();

int sc_is_empty(); // return 1 if sc is empty, 0 otherwise
int sc_is_full(); // return 1 if sc is full, 0 otherwise

int sc_pop_victim(); // give frame id that is to be evicted, -1 when sc is empty
int sc_push_frame(int frame); // push frame to sc, -1 on error (sc is full)
int sc_remove_frame(int frame); // remove frame from sc (on frame free), 1 on success
int sc_access_frame(int frame); // access frame, 0 on success

void sc_print_debug();