/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <paging.h>
#include <proc.h>

#define VGETMEM_DEBUG (1 & GLOBAL_DEBUG)

#if VGETMEM_DEBUG
#include <stdio.h>
#define VGETMEM_DEBUG_PRINTF(...)                                              \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [VGETMEM] | ");                                 \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define VGETMEM_DEBUG_PRINTF(...)                                              \
  do {                                                                         \
  } while (0)
#endif

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD *vgetmem(unsigned int nbytes) {
  if (nbytes == 0) {
    VGETMEM_DEBUG_PRINTF("Requested bytes is 0. Cannot allocate 0 bytes\n");
    return (WORD *)SYSERR;
  }

  STATWORD ps;
  disable(ps);
  struct mblock *current_block, *previous_block, *remaining_block;
  struct mblock *virtual_mem_list;

  virtual_mem_list = proctab[currpid].vmemlist;
  if (virtual_mem_list->mnext == NULL) {
    VGETMEM_DEBUG_PRINTF("Allocation failed: Empty memory list\n");
    restore(ps);
    return (WORD *)SYSERR;
  }

  unsigned int roundedNbytes = (unsigned int)roundmb(nbytes);
  VGETMEM_DEBUG_PRINTF("Requested bytes: %d, Rounded bytes: %d\n", nbytes,
                       roundedNbytes);

  previous_block = virtual_mem_list;
  current_block = virtual_mem_list->mnext;

  while (current_block != NULL) {

    if (current_block->mlen == roundedNbytes) {
      previous_block->mnext = current_block->mnext;
      VGETMEM_DEBUG_PRINTF("Allocation successful: Exact size match found\n");
      restore(ps);
      return (WORD *)current_block;
    }

    if (current_block->mlen > roundedNbytes) {
      remaining_block =
          (struct mblock *)((unsigned)current_block + roundedNbytes);
      previous_block->mnext = remaining_block;
      remaining_block->mnext = current_block->mnext;
      remaining_block->mlen = current_block->mlen - roundedNbytes;
      VGETMEM_DEBUG_PRINTF("Allocation successful: Larger block split. Found "
                           "block of length %d, remaining block of length %d\n",
                           current_block->mlen, remaining_block->mlen);
      restore(ps);
      return (WORD *)current_block;
    }
    previous_block = current_block;
    current_block = current_block->mnext;
  }

  VGETMEM_DEBUG_PRINTF("Allocation failed: No suitable block found\n");
  restore(ps);
  return (WORD *)SYSERR;
}
