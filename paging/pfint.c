/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>

#define PFINT_DEBUG 1

#if PFINT_DEBUG
#include <stdio.h>
#define PFINT_DEBUG_PRINTF(...) do { kprintf("\033[32m[DEBUG] | [PFINT] | "); kprintf(__VA_ARGS__); kprintf("\033[0m"); } while(0)
#else
#define PFINT_DEBUG_PRINTF(...)
#endif

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR (interrupt service routine)
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

  PFINT_DEBUG_PRINTF("Page fault ISR called\n");

  return OK;
}


