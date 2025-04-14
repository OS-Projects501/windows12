/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

#define PFINT_DEBUG (1 & GLOBAL_DEBUG)

#if PFINT_DEBUG
#include <stdio.h>
#define PFINT_DEBUG_PRINTF(...)                                                \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [PFINT] | ");                                   \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define PFINT_DEBUG_PRINTF(...)                                                \
  do {                                                                         \
  } while (0)
#endif

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint() {
  STATWORD ps;
  disable(ps);
  unsigned long linear_addr = read_cr2();

  PFINT_DEBUG_PRINTF("pfint: begin [Fault for 0x%x]\n", linear_addr);
  int store, pageth;

  if (bsm_lookup(currpid, linear_addr, &store, &pageth) == SYSERR) {
    PFINT_DEBUG_PRINTF("pfint: bsm_lookup failed. Killing process %d\n",
                       currpid);
    kill(currpid);
    restore(ps);
    return SYSERR;
  }

  // we have found the backing store and the page that is supposed to be loaded
  // back into memory
  register unsigned int pd_offset = ((virt_addr_t *)(&linear_addr))->pd_offset;
  register unsigned int pt_offset = ((virt_addr_t *)(&linear_addr))->pt_offset;
  register unsigned int pg_offset = ((virt_addr_t *)(&linear_addr))->pg_offset;
  PFINT_DEBUG_PRINTF("pfint: pd_offset: %d, pt_offset: %d, pg_offset: %d\n",
                     pd_offset, pt_offset, pg_offset);

  pd_t *pd_addr = (pd_t *)(proctab[currpid].pdbr);
  pt_t *pt_addr;
  int frame_id;

  // Load page directory if it does not exist
  // Then get page table's base address
  if (pd_addr[pd_offset].pd_pres == FALSE) {
    frame_id = get_page_table(currpid);
    if (frame_id == SYSERR) {
      PFINT_DEBUG_PRINTF("pfint: get_page_table failed. Killing process %d\n", currpid);
      kill(currpid);
      restore(ps);
      return SYSERR;
    }
    pt_addr = (pt_t *)(frame_address(frame_id));
    pd_addr[pd_offset].pd_pres = TRUE;
    pt_addr[pt_offset].pt_global = FALSE;
    pd_addr[pd_offset].pd_base = frame_id + FRAME0;
  } else {
    pt_addr = (pt_t *)(pd_addr[pd_offset].pd_base * NBPG);
  }

  // Get empty frame we can use to load the page into
  // Kill process on fail
  if (get_frm(&frame_id) == SYSERR) {
    PFINT_DEBUG_PRINTF("pfint: get_frm failed. Killing process %d\n", currpid);
    kill(currpid);
    restore(ps);
    return (SYSERR);
  }

  // Make replacement policy aware of the new frame
  rp_push_frame(frame_id);

  // Update frame table
  frm_tab[frame_id].fr_status = FRM_MAPPED;
  frm_tab[frame_id].fr_pid = currpid;
  frm_tab[frame_id].fr_vpno = linear_addr >> 12;
  frm_tab[frame_id].fr_type = FR_PAGE;
  frm_tab[frame_id].fr_dirty = FALSE;

  // Get backing store frame address and load page
  unsigned long frm_addr = frame_address(frame_id);
  read_bs((char *)frm_addr, store, pageth);

  // Update page table
  pt_addr[pt_offset].pt_pres = TRUE;
  pt_addr[pt_offset].pt_base = FRAME0 + frame_id;
  pt_addr[pt_offset].pt_global = FALSE;

  restore(ps);
  return OK;
}
