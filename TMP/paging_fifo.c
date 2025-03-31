#include "paging_fifo.h"

static int fifo[MAX_FIFO_SIZE];
static int start_idx = 0;
static int size = 0;

int fifo_size() { return size; }

bool fifo_is_empty() { return size == 0; }

bool fifo_is_full() { return size == MAX_FIFO_SIZE; }

void fifo_enqueue(int data) {
  if (fifo_is_full()) {
    start_idx = (start_idx + 1) % MAX_FIFO_SIZE;
    size--;
  }
  int write_idx = (start_idx + size) % MAX_FIFO_SIZE;
  fifo[write_idx] = data;
  size++;
}

int fifo_dequeue() {
  if (fifo_is_empty()) {
    return -1;
  }
  int data = fifo[start_idx];
  start_idx = (start_idx + 1) % MAX_FIFO_SIZE;
  size--;
  return data;
}

int fifo_index(int i) { return (start_idx + i) % MAX_FIFO_SIZE; }
