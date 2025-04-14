#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

#define RELEASE_BS_DEBUG (1 & GLOBAL_DEBUG)

#if RELEASE_BS_DEBUG
#include <stdio.h>
#define RELEASE_BS_DEBUG_PRINTF(...)                                           \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [RELEASE_BS] | ");                              \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define RELEASE_BS_DEBUG_PRINTF(...)                                           \
  do {                                                                         \
  } while (0)
#endif

SYSCALL release_bs(bsd_t bs_id) {
  STATWORD ps;
  disable(ps);

  if (bs_id < 0 || bs_id >= N_BS) {
    RELEASE_BS_DEBUG_PRINTF(
        "Invalid backing store ID: %d\n Choose between 0 and %d", bs_id,
        N_BS - 1);
    restore(ps);
    return SYSERR;
  }

  if (bsm_tab[bs_id].bs_status != BSM_MAPPED) {
    RELEASE_BS_DEBUG_PRINTF("Backing store %d is not mapped", bs_id);
    restore(ps);
    return SYSERR;
  }

  if (bsm_tab[bs_id].bs_pid != currpid) {
    RELEASE_BS_DEBUG_PRINTF("Backing store %d is not owned by process %d",
                            bs_id, currpid);
    restore(ps);
    return SYSERR;
  }

  // Reset backing store attributes
  bsm_tab[bs_id].bs_pid = -1;
  bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
  bsm_tab[bs_id].bs_npages = N_BS_FRAMES;

  restore(ps);
  return SYSERR;
}
