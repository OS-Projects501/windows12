#ifndef PAGING_FIFO_H
#define PAGING_FIFO_H

// project
#include "paging.h"

// stdlib
#include <stdbool.h>
#include <stdio.h>

#define MAX_FIFO_SIZE NFRAMES

// Get the current number of elements in the FIFO
int fifo_size();

bool fifo_is_empty();
bool fifo_is_full();
void fifo_enqueue(int data);
int fifo_dequeue();

// Get the index of the i-th element in the FIFO
int fifo_index(int i);

#endif // PAGING_FIFO_H
