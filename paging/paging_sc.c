#include "paging_sc.h"

static int sc[MAX_SC_SIZE];
static bool reference_bits[MAX_SC_SIZE];
static int write_idx = 0;
static int size = 0;

int sc_size() { return size; }

bool sc_is_empty() { return size == 0; }

bool sc_is_full() { return size == MAX_SC_SIZE; }

void sc_enqueue(int data) {
  if (sc_is_full()) {
    while (reference_bits[write_idx]) {
      reference_bits[write_idx] = false;
      write_idx = (write_idx + 1) % MAX_SC_SIZE;
    }
    size--;
  }

  sc[write_idx] = data;
  reference_bits[write_idx] = true;
  write_idx = (write_idx + 1) % MAX_SC_SIZE;
  size++;
}

void sc_access(int data) {
  for (int i = 0; i < size; i++) {
    if (sc[i] == data) {
      reference_bits[i] = true;
      break;
    }
  }
}
