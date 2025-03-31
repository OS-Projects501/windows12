/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

#define FRAME_DEBUG 1

#if FRAME_DEBUG
#include <stdio.h>
#define FRAME_DEBUG_PRINTF(...) do { kprintf("\033[32m[DEBUG] | [FRAME] | "); kprintf(__VA_ARGS__); kprintf("\033[0m"); } while(0)
#else
#define FRAME_DEBUG_PRINTF(...)
#endif

static int get_free_frame_SC() {
  FRAME_DEBUG_PRINTF("Getting a free frame using SC policy\n");

    // First try to find an unmapped frame
    // If we find one, then return it right away
    for (int i = 0; i < NFRAMES; i++) {
        if (frm_tab[i].fr_status == FRM_UNMAPPED) {
            FRAME_DEBUG_PRINTF("Found unmapped frame %d\n", i);
            return i;
        }
    }

    // If no unmapped frame found, use SC policy to select a victim frame
    // We'll use the first mapped frame as the victim (SC)
    for (int i = 0; i < NFRAMES; i++) {
        if (frm_tab[i].fr_status == FRM_MAPPED) {
            // TODO: Implement SC policy
            FRAME_DEBUG_PRINTF("Selected frame %d as victim using SC\n", i);
            return i;
        }
    }

    // If we get here, something is wrong - all frames should be either mapped or unmapped
    FRAME_DEBUG_PRINTF("ERROR: No valid frame found\n");
    return -1;
}

static int get_free_frame_FIFO() {
    FRAME_DEBUG_PRINTF("Getting a free frame using FIFO policy\n");

    // First try to find an unmapped frame
    // If we find one, then return it right away
    for (int i = 0; i < NFRAMES; i++) {
        if (frm_tab[i].fr_status == FRM_UNMAPPED) {
            FRAME_DEBUG_PRINTF("Found unmapped frame %d\n", i);
            return i;
        }
    }

    // If no unmapped frame found, use FIFO policy to select a victim frame
    // We'll use the first mapped frame as the victim (FIFO)
    for (int i = 0; i < NFRAMES; i++) {
        if (frm_tab[i].fr_status == FRM_MAPPED) {
            // TODO: Implement FIFO policy
            FRAME_DEBUG_PRINTF("Selected frame %d as victim using FIFO\n", i);
            return i;
        }
    }

    // If we get here, something is wrong - all frames should be either mapped or unmapped
    FRAME_DEBUG_PRINTF("ERROR: No valid frame found\n");
    return -1;
}

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  STATWORD ps;
  disable(ps);

  FRAME_DEBUG_PRINTF("Initializing frame table\n");

  for (int i = 0; i < NFRAMES; i++) {
    frm_tab[i].fr_status = FRM_UNMAPPED;
    frm_tab[i].fr_pid = -1; // -1 means no process is using this frame
    frm_tab[i].fr_vpno = -1; // -1 means unmapped
    frm_tab[i].fr_refcnt = 0; // 0 means no references to this frame, if this is 0, fr_pid should be -1
    frm_tab[i].fr_type = FR_PAGE;
    frm_tab[i].fr_dirty = FR_NOT_DIRTY;
  }

  FRAME_DEBUG_PRINTF("Frame table initialized\n");

  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  STATWORD ps;
  disable(ps);

  // Make sure avail is not a null pointer
  if (avail == NULL) {
    kprintf("ERROR: Invalid avail pointer: %s:%d\n", __FILE__, __LINE__);
    restore(ps);
    return SYSERR;
  }

  // Read what policy we're using
  int policy = grpolicy();
  FRAME_DEBUG_PRINTF("Getting a free frame. Using policy %s(%d)\n", as_str(policy), policy);

  // Get frame using replacement policy
  switch (policy) {
    case SC:
      *avail = get_free_frame_SC();
      break;
    case FIFO:
      *avail = get_free_frame_FIFO();
      break;
    default:
      *avail = -1;
      FRAME_DEBUG_PRINTF("ERROR: Invalid policy %s(%d)\n", as_str(policy), policy);
  }

  int ret = (*avail == -1) ? SYSERR : OK;
  restore(ps);
  return ret;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  STATWORD ps;
  disable(ps);

  FRAME_DEBUG_PRINTF("Freeing frame %d\n", i);

  restore(ps);
  return OK;
}



