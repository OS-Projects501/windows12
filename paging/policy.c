/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>

#define POLICY_DEBUG 1

#if POLICY_DEBUG
#include <stdio.h>
#define POLICY_DEBUG_PRINTF(...) do { kprintf("\033[32m[DEBUG] | [POLICY] | "); kprintf(__VA_ARGS__); kprintf("\033[0m"); } while(0)
#else
#define POLICY_DEBUG_PRINTF(...)
#endif


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

static Bool is_valid_policy(int policy) {
  return (Bool)(policy == SC || policy == FIFO);
}

extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  STATWORD ps;
  disable(ps);

  if (!is_valid_policy(policy)) {
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
SYSCALL grpolicy()
{
  POLICY_DEBUG_PRINTF("Getting policy. Returning %s(%d)\n", as_str(page_replace_policy), page_replace_policy);
  return page_replace_policy;
}
