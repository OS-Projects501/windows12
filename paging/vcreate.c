/* vcreate.c - vcreate */

#include <conf.h>
#include <i386.h>
#include <io.h>
#include <kernel.h>
#include <mem.h>
#include <paging.h>
#include <proc.h>
#include <sem.h>

#define VCREATE_DEBUG (1 & GLOBAL_DEBUG)

#if VCREATE_DEBUG
#include <stdio.h>
#define VCREATE_DEBUG_PRINTF(...)                                              \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [VCREATE] | ");                                 \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define VCREATE_DEBUG_PRINTF(...)                                              \
  do {                                                                         \
  } while (0)
#endif

/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(int procaddr, int ssize, int hsize, int priority, char *name,
                int nargs, long args) {
  STATWORD ps;
  disable(ps);

  int pid = create((int *)((long)procaddr), ssize, priority, name, nargs, args);
  VCREATE_DEBUG_PRINTF("PID: %d", pid);

  if (pid == SYSERR) {
    VCREATE_DEBUG_PRINTF("Error creating process");
    restore(ps);
    return SYSERR;
  }

  // get backing store
  int store = -1;
  if (get_bsm(&store) == SYSERR) {
    VCREATE_DEBUG_PRINTF("Error getting backing store");
    restore(ps);
    return SYSERR;
  }

  // map backing store
  // 4096, that's where the virtual address space starts for this process
  if (bsm_map(pid, TOP_FRAME, store, hsize) == SYSERR) {
    VCREATE_DEBUG_PRINTF("Error mapping backing store");
    restore(ps);
    return SYSERR;
  }

  // store process info
  proctab[pid].vhpnpages = hsize;
  proctab[pid].vhpno = TOP_FRAME;
  proctab[pid].store = store;
  proctab[pid].vmemlist->mnext =
      (struct mblock *)((unsigned long)(TOP_FRAME * NBPG));
  proctab[pid].vmemlist->mlen = 0;

  // Initially, heap is empty = one big free block with length hsize * NBPG
  struct mblock *mptr = (struct mblock *)backing_store_address(store);
  mptr->mnext = NULL;
  mptr->mlen = hsize * NBPG;

  restore(ps);
  return pid;
}
