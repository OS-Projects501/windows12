#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

#define GET_BS_DEBUG (1 & GLOBAL_DEBUG)

#if GET_BS_DEBUG
#include <stdio.h>
#define GET_BS_DEBUG_PRINTF(...)                                               \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [GET_BS] | ");                                  \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define GET_BS_DEBUG_PRINTF(...)                                               \
  do {                                                                         \
  } while (0)
#endif

int get_bs(bsd_t bs_id, unsigned int npages) {
  if (bs_id < 0 || bs_id >= N_BS) {
    GET_BS_DEBUG_PRINTF(
        "Invalid backing store ID: %d\n Choose between 0 and %d", bs_id,
        N_BS - 1);
    return SYSERR;
  }

  if ((npages <= 0) || (npages > N_BS_FRAMES)) {
    GET_BS_DEBUG_PRINTF(
        "Invalid number of pages requested: %d\n Choose between 1 and %d",
        npages, N_BS_FRAMES);
    return SYSERR;
  }

  STATWORD ps;
  disable(ps);

  if (bsm_tab[bs_id].bs_status == BSM_MAPPED) {
    GET_BS_DEBUG_PRINTF("Backing store %d is already mapped", bs_id);
    restore(ps);
    return bsm_tab[bs_id].bs_npages;
  }

  GET_BS_DEBUG_PRINTF("Mapping backing store %d with %d pages", bs_id, npages);
  bsm_tab[bs_id].bs_status = BSM_MAPPED;
  bsm_tab[bs_id].bs_pid = currpid;
  bsm_tab[bs_id].bs_npages = npages;
  restore(ps);
  return npages;
}
