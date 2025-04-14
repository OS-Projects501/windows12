/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

#define MMAP_DEBUG (1 & GLOBAL_DEBUG)

#if MMAP_DEBUG
#include <stdio.h>
#define MMAP_DEBUG_PRINTF(...)                                                 \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [MMAP] | ");                                    \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define MMAP_DEBUG_PRINTF(...)                                                 \
  do {                                                                         \
  } while (0)
#endif

/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t store, int npages) {

  MMAP_DEBUG_PRINTF("store: %d, npages: %d mapping to vpno 0x%x for pid %d\n",
                    store, npages, virtpage, currpid);

  if (npages <= 0) {
    MMAP_DEBUG_PRINTF("Invalid number of pages: %d. Has to be positive\n",
                      npages);
    return SYSERR;
  }

  STATWORD ps;
  disable(ps);

  if ((virtpage >= TOP_FRAME) && (bsm_tab[store].bs_status == BSM_MAPPED) &&
      (bsm_tab[store].bs_pid == currpid) &&
      (npages <= bsm_tab[store].bs_npages)) {

    MMAP_DEBUG_PRINTF("Mapping to BS %d. virtpage: 0x%x\n", store, virtpage);
    bsm_tab[store].bs_vpno = virtpage;
    restore(ps);
    return OK;
  } else {
    MMAP_DEBUG_PRINTF("Failed to map to BS %d. virtpage: 0x%x\n", store,
                      virtpage);
    restore(ps);
    return SYSERR;
  }
}

/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage) {
  MMAP_DEBUG_PRINTF("Unmapping vpno: 0x%x for pid: %d\n", virtpage, currpid);

  if (virtpage < TOP_FRAME) {
    MMAP_DEBUG_PRINTF("Invalid virtual page number: 0x%x. Must be at least "
                      "%d. Reserved for global\n",
                      virtpage, TOP_FRAME);
    return SYSERR;
  }

  STATWORD ps;
  disable(ps);
  pd_t *pd = (pd_t *)(proctab[currpid].pdbr);
  /*
    typedef struct {
    unsigned int pg_offset : 12; virtpage (this is what we want)
    unsigned int pt_offset : 10; virtpage
    unsigned int pd_offset : 10; already gone
  } virt_addr_t;  
  */
  unsigned long pg_offset = virtpage >> 10;

  for (int i = 0; i < N_BS; i++) {
    Bool is_mapped = (bsm_tab[i].bs_status == BSM_MAPPED);
    Bool is_owned = (bsm_tab[i].bs_pid == currpid);
    Bool is_correct_vpno = (bsm_tab[i].bs_vpno == virtpage);
    Bool pd_present = (pd[pg_offset].pd_pres == TRUE);

    if (is_mapped && is_owned && is_correct_vpno && pd_present) {
      MMAP_DEBUG_PRINTF("Unmapping page table for vpno: 0x%x\n", virtpage);
      free_frm(pd[pg_offset].pd_base - FRAME0);
      pd[pg_offset].pd_base = FALSE;
      pd[pg_offset].pd_global = FALSE;
      pd[pg_offset].pd_pres = FALSE;
      bsm_tab[i].bs_vpno = -1;
      MMAP_DEBUG_PRINTF("Unmapping succeeded!\n");
      restore(ps);
      return OK;
    }
  }

  MMAP_DEBUG_PRINTF("Unmapping failed!\n");

  restore(ps);
  return SYSERR;
}
