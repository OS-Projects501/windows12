/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

#define BSM_DEBUG (1 & GLOBAL_DEBUG)

#if BSM_DEBUG
#include <stdio.h>
#define BSM_DEBUG_PRINTF(...)                                                  \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [BSM] | ");                                     \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define BSM_DEBUG_PRINTF(...)                                                  \
  do {                                                                         \
  } while (0)
#endif

bs_map_t bsm_tab[N_BS];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm() {
  STATWORD ps;
  disable(ps);
  for (int i = 0; i < N_BS; i++) {
    bsm_tab[i].bs_status = BSM_UNMAPPED;
    bsm_tab[i].bs_pid = -1;
    bsm_tab[i].bs_vpno = 0;
    bsm_tab[i].bs_npages = N_BS_FRAMES; /* Default size per backing store */
    bsm_tab[i].bs_sem = 0;
    bsm_tab[i].bs_private = FALSE;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int *avail) {
  if (avail == NULL) {
    BSM_DEBUG_PRINTF("Invalid availability pointer");
    return SYSERR;
  }
  STATWORD ps;
  disable(ps);

  // find the first unmapped backing store
  for (int i = 0; i < N_BS; i++) {
    if (bsm_tab[i].bs_status == BSM_UNMAPPED) {
      *avail = i;
      BSM_DEBUG_PRINTF("Found free backing store %d", i);
      restore(ps);
      return OK;
    }
  }

  BSM_DEBUG_PRINTF("No free backing store found");
  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int bs_id) {
  STATWORD ps;
  disable(ps);

  if (bs_id < 0 || bs_id >= N_BS) {
    BSM_DEBUG_PRINTF("Invalid backing store ID: %d\n Choose between 0 and %d",
                     bs_id, N_BS - 1);
    restore(ps);
    return SYSERR;
  }

  if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED) {
    BSM_DEBUG_PRINTF("Backing store %d is already free. This is an error",
                     bs_id);
    restore(ps);
    return SYSERR;
  }

  bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
  bsm_tab[bs_id].bs_pid = -1;
  bsm_tab[bs_id].bs_vpno = 0;
  bsm_tab[bs_id].bs_npages = N_BS_FRAMES;
  bsm_tab[bs_id].bs_sem = 0;
  bsm_tab[bs_id].bs_private = FALSE;
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - Look up a backing store mapping for a given process and virtual
 *address
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int *store, int *pageth) {
  STATWORD ps;
  disable(ps);

  unsigned long virtual_page_number = vaddr >> 12; // 12 = log2(4096)

  for (int i = 0; i < N_BS; i++) {
    Bool is_mapped = bsm_tab[i].bs_status == BSM_MAPPED;
    Bool is_owned_by_pid = bsm_tab[i].bs_pid == pid;
    Bool is_in_range =
        bsm_tab[i].bs_vpno <= virtual_page_number &&
        (bsm_tab[i].bs_vpno + bsm_tab[i].bs_npages) > virtual_page_number;
    if (is_mapped && is_owned_by_pid && is_in_range) {
      *pageth = virtual_page_number - bsm_tab[i].bs_vpno;
      *store = i;
      BSM_DEBUG_PRINTF("bsm_lookup: Found backing store %d for process %d at "
                       "virtual page number %d\n",
                       i, pid, virtual_page_number);
      restore(ps);
      return OK;
    }
  }

  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages) {
  STATWORD ps;
  disable(ps);

  if (pid < 0 || pid >= NPROC) {
    BSM_DEBUG_PRINTF("Invalid process ID: %d\n Choose between 0 and %d", pid,
                     NPROC - 1);
    restore(ps);
    return SYSERR;
  }

  if (source < 0 || source >= N_BS) {
    BSM_DEBUG_PRINTF("Invalid backing store ID: %d\n Choose between 0 and %d",
                     source, N_BS - 1);
    restore(ps);
    return SYSERR;
  }

  bsm_tab[source].bs_pid = pid;
  bsm_tab[source].bs_vpno = vpno;
  bsm_tab[source].bs_npages = npages;
  bsm_tab[source].bs_status = BSM_MAPPED;

  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag) {
  STATWORD ps;
  disable(ps);

  if (pid < 0 || pid >= NPROC) {
    BSM_DEBUG_PRINTF("Invalid process ID: %d\n Choose between 0 and %d", pid,
                     NPROC - 1);
    restore(ps);
    return SYSERR;
  }

  restore(ps);
  return OK;
}
