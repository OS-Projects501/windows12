#include "paging_sc.h"

#define SC_DEBUG (1 & GLOBAL_DEBUG)

#if SC_DEBUG
#include <stdio.h>
#define SC_DEBUG_PRINTF(...)                                                   \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [SC] | ");                                      \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define SC_DEBUG_PRINTF(...)                                                   \
  do {                                                                         \
  } while (0)
#endif

typedef struct {
  int frame;
  int reference_bit;
  int present;
} Node;

static Node sc[MAX_SC_SIZE];
static int head_idx = 0;
static int initialized = 0;

void sc_init() {
  SC_DEBUG_PRINTF("Initializing SC\n");
  if (initialized) {
    return;
  }
  for (int i = 0; i < MAX_SC_SIZE; i++) {
    sc[i].present = 0;
    sc[i].reference_bit = 0;
    sc[i].frame = -1;
  }
}

static int size() {
  int count = 0;
  for (int i = 0; i < MAX_SC_SIZE; i++) {
    if (sc[i].present) {
      count++;
    }
  }
  return count;
}

int sc_is_empty() { return size() == 0; }

int sc_is_full() { return size() == MAX_SC_SIZE; }

int sc_pop_victim() {
  if (sc_is_empty()) {
    return -1;
  }

  while (1) {
    if (!sc[head_idx].reference_bit && sc[head_idx].present) {
      break;
    }
    sc[head_idx].reference_bit = 0;
    head_idx = (head_idx + 1) % MAX_SC_SIZE;
  }

  int victim = sc[head_idx].frame;
  sc[head_idx].present = 0;
  SC_DEBUG_PRINTF("SC: pop_victim: %d\n", victim);
  return victim;
}

int sc_push_frame(int frame) {
  if (sc_is_full()) {
    return -1;
  }

  while (1) {
    if (!sc[head_idx].present) {
      break;
    }
    head_idx = (head_idx + 1) % MAX_SC_SIZE;
  }

  sc[head_idx].frame = frame;
  sc[head_idx].reference_bit = 1;
  sc[head_idx].present = 1;
  head_idx = (head_idx + 1) % MAX_SC_SIZE;
  return 0;
}

int sc_remove_frame(int frame) {
  if (sc_is_empty()) {
    return 1;
  }

  for (int i = 0; i < MAX_SC_SIZE; i++) {
    if (sc[i].frame == frame) {
      sc[i].present = 0;
    }
  }
  return 0;
}

int sc_access_frame(int frame) {
  for (int i = 0; i < MAX_SC_SIZE; i++) {
    if (sc[i].frame == frame && sc[i].present) {
      sc[i].reference_bit = 1;
    }
  }
  return 0;
}

void sc_print_debug() {
  SC_DEBUG_PRINTF("-----------------------------------\n");
  SC_DEBUG_PRINTF("SC: size: %d\n", size());
  for (int i = 0; i < MAX_SC_SIZE; i++) {
    if (sc[i].present) {
      SC_DEBUG_PRINTF("SC: idx: %d, frame: %d, present: %d, reference_bit: %d\n",
                    i, sc[i].frame, sc[i].present, sc[i].reference_bit);
    }
  }
  SC_DEBUG_PRINTF("-----------------------------------\n");
}
