/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <paging.h>
#include <proc.h>

#define VFREEMEM_DEBUG (1 & GLOBAL_DEBUG)

#if VFREEMEM_DEBUG
#include <stdio.h>
#define VFREEMEM_DEBUG_PRINTF(...)                                             \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [VFREEMEM] | ");                                \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define VFREEMEM_DEBUG_PRINTF(...)                                             \
  do {                                                                         \
  } while (0)
#endif

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL vfreemem(struct mblock *block, unsigned int size) {
  STATWORD ps;
  disable(ps);

  if (size == 0) {
    VFREEMEM_DEBUG_PRINTF("Requested size is 0. Cannot free 0 bytes\n");
    restore(ps);
    return SYSERR;
  }

  unsigned long block_addr = (unsigned long)block;
  unsigned long min_addr = TOP_FRAME * NBPG;
  unsigned long max_addr = (proctab[currpid].vhpnpages * NBPG) + min_addr;

  Bool is_valid_address = (block_addr >= min_addr && block_addr <= max_addr);
  if (!is_valid_address) {
    VFREEMEM_DEBUG_PRINTF(
        "Invalid block address: %lu. Address out of valid range [%lu, %lu]\n",
        block_addr, min_addr, max_addr);
    restore(ps);
    return SYSERR;
  }

  unsigned int rounded_size = (unsigned int)roundmb(size);
  VFREEMEM_DEBUG_PRINTF(
      "Freeing block at address %lu with size %d (rounded to %d)\n", block_addr,
      size, rounded_size);

  struct mblock *memlist = proctab[currpid].vmemlist;
  struct mblock *prev = memlist;
  struct mblock *curr = memlist->mnext;
  unsigned long top;

  while (curr != NULL && curr < block) {
    prev = curr;
    curr = curr->mnext;
  }

  top = (unsigned long)prev + prev->mlen;
  if ((top > block_addr && prev != memlist) ||
      (curr != NULL && (block_addr + rounded_size) > (unsigned long)curr)) {
    VFREEMEM_DEBUG_PRINTF(
        "Invalid block position: Overlap detected with adjacent blocks\n");
    restore(ps);
    return SYSERR;
  }

  if (prev != memlist && top == block_addr) {
    prev->mlen += rounded_size;
    VFREEMEM_DEBUG_PRINTF("Merged with previous block. New length: %d\n",
                          prev->mlen);
  } else {
    block->mlen = rounded_size;
    block->mnext = curr;
    prev->mnext = block;
    prev = block;
    VFREEMEM_DEBUG_PRINTF("Inserted as new block with length: %d\n",
                          rounded_size);
  }

  if (curr != NULL &&
      ((unsigned long)prev + prev->mlen) == (unsigned long)curr) {
    prev->mlen += curr->mlen;
    prev->mnext = curr->mnext;
    VFREEMEM_DEBUG_PRINTF("Merged with next block. New length: %d\n",
                          prev->mlen);
  }

  VFREEMEM_DEBUG_PRINTF("Memory block freed successfully\n");
  restore(ps);
  return OK;
}
