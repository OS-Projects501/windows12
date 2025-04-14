#include "paging_fifo.h"

#define FIFO_DEBUG (1 & GLOBAL_DEBUG)

#if FIFO_DEBUG
#include <stdio.h>
#define FIFO_DEBUG_PRINTF(...)                                                 \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [FIFO] | ");                                    \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define FIFO_DEBUG_PRINTF(...) do {} while (0)
#endif

static int fifo[MAX_FIFO_SIZE];
static int start_idx = 0;
static int size = 0;
static int initialized = 0;

void fifo_init() {
  FIFO_DEBUG_PRINTF("Initializing FIFO\n");
  if (initialized) {
    return;
  }
  initialized = 1;
}

int fifo_is_empty() { return size == 0; }

int fifo_is_full() { return size == MAX_FIFO_SIZE; }

int fifo_pop_victim() {
  if (fifo_is_empty()) {
    return -1;
  }
  int victim = fifo[start_idx];
  start_idx = (start_idx + 1) % MAX_FIFO_SIZE;
  size--;
  FIFO_DEBUG_PRINTF("FIFO: pop_victim: %d\n", victim);
  return victim;
}

int fifo_push_frame(int frame) {
  if (fifo_is_full()) {
    return -1;
  }
  fifo[(start_idx + size) % MAX_FIFO_SIZE] = frame;
  size++;
  return 0;
}

int fifo_remove_frame(int frame) {

  // nothing to remove
  if (fifo_is_empty())
    return 1;

  int new_size = 0;
  for (int i = 0; i < size; i++) {
    if (fifo[(start_idx + i) % MAX_FIFO_SIZE] == frame)
      continue;
    fifo[(start_idx + new_size) % MAX_FIFO_SIZE] =
        fifo[(start_idx + i) % MAX_FIFO_SIZE];
    new_size++;
  }

  size = new_size;
  return 0;
}

void fifo_print_debug() {
  FIFO_DEBUG_PRINTF("FIFO: size: %d\n", size);
  for (int i = 0; i < size; i++) {
    FIFO_DEBUG_PRINTF("FIFO: idx: %d, frame: %d\n",
                      (start_idx + i) % MAX_FIFO_SIZE,
                      fifo[(start_idx + i) % MAX_FIFO_SIZE]);
  }
}
