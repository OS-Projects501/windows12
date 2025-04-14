/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <paging_fifo.h>
#include <paging_sc.h>

#define POLICY_DEBUG (1 & GLOBAL_DEBUG)

#if POLICY_DEBUG
#include <stdio.h>
#define POLICY_DEBUG_PRINTF(...)                                               \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [POLICY] | ");                                  \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define POLICY_DEBUG_PRINTF(...)
#endif

static int page_replace_policy = FIFO;

const char *as_str(int policy) {
  switch (policy) {
  case SC:
    return "SC";
  case FIFO:
    return "FIFO";
  default:
    return "UNKNOWN";
  }
}

extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy) {
  STATWORD ps;
  disable(ps);

  Bool is_valid_policy = (policy == SC || policy == FIFO);
  if (!is_valid_policy) {
    POLICY_DEBUG_PRINTF("Invalid policy %s(%d)\n", as_str(policy), policy);
    restore(ps);
    return SYSERR;
  }

  POLICY_DEBUG_PRINTF("Setting policy to %s(%d)\n", as_str(policy), policy);
  page_replace_policy = policy;

  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy() {
  POLICY_DEBUG_PRINTF("Getting policy. Returning %s(%d)\n",
                      as_str(page_replace_policy), page_replace_policy);
  return page_replace_policy;
}

int rp_push_frame(int frm_id) {
  STATWORD ps;
  disable(ps);
  int ret = SYSERR;
  switch (page_replace_policy) {
  case SC:
    ret = sc_push_frame(frm_id);
    break;
  case FIFO:
    ret = fifo_push_frame(frm_id);
    break;
  default:
    ret = SYSERR;
    break;
  }
  restore(ps);
  return ret;
}

int rp_pop_victim() {
  STATWORD ps;
  disable(ps);
  int ret = SYSERR;
  switch (page_replace_policy) {
  case SC:
    ret = sc_pop_victim();
    break;
  case FIFO:
    ret = fifo_pop_victim();
    break;
  default:
    ret = SYSERR;
    break;
  }
  restore(ps);
  return ret;
}

int rp_remove_frame(int frm_id) {
  STATWORD ps;
  disable(ps);
  int ret = SYSERR;
  switch (page_replace_policy) {
  case SC:
    ret = sc_remove_frame(frm_id);
    break;
  case FIFO:
    ret = fifo_remove_frame(frm_id);
    break;
  default:
    ret = SYSERR;
    break;
  }
  restore(ps);
  return ret;
}