#ifndef PAGING_SC_H
#define PAGING_SC_H

// project
#include "paging.h"

// stdlib
#include <stdbool.h>
#include <stdio.h>

#define MAX_SC_SIZE NFRAMES

// current number of elements in the second-chance queue
int sc_size();

bool sc_is_empty();
bool sc_is_full();
void sc_enqueue(int data);
void sc_access(int data);

#endif // PAGING_SC_H
