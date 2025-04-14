/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

#define FRAME_DEBUG (1 & GLOBAL_DEBUG)

#if FRAME_DEBUG
#include <stdio.h>
#define FRAME_DEBUG_PRINTF(...)                                                \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [FRAME] | ");                                   \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define FRAME_DEBUG_PRINTF(...)                                                \
  do {                                                                         \
  } while (0)
#endif

fr_map_t frm_tab[NFRAMES];
global_page_table_t gpt;

/*-------------------------------------------------------------------------
 * init_frm - initialize frame table
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm() {
  STATWORD ps;
  disable(ps);
  for (int i = 0; i < NFRAMES; i++) {
    frm_tab[i].fr_status = FRM_UNMAPPED;
    frm_tab[i].fr_pid = -1;
    frm_tab[i].fr_vpno = -1;
    frm_tab[i].fr_refcnt = 0;
    frm_tab[i].fr_type = -1;
    frm_tab[i].fr_dirty = -1;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * init_global_page_table - initialize global page tables (first 4 page tables,
 * 1024 entries each = 4096 entries total)
 *-------------------------------------------------------------------------
 */
int init_global_page_table() {
  STATWORD ps;
  disable(ps);

  int frm_id;
  pt_t *page_table;
  for (int i = 0; i < 4; i++) {
    frm_id = get_page_table(NULLPROC);
    if (frm_id == SYSERR) {
      restore(ps);
      return SYSERR;
    }
    switch (i) {
    case 0:
      gpt.gpt1_idx = FRAME0 + frm_id;
      break;
    case 1:
      gpt.gpt2_idx = FRAME0 + frm_id;
      break;
    case 2:
      gpt.gpt3_idx = FRAME0 + frm_id;
      break;
    case 3:
      gpt.gpt4_idx = FRAME0 + frm_id;
      break;
    }

    for (int j = 0; j < PAGESIZE; j++) {
      page_table = (pt_t *)(frame_address(frm_id) + (j * sizeof(pt_t)));
      page_table->pt_pres = TRUE;
      page_table->pt_global = TRUE;
      page_table->pt_base = (frm_id * PAGESIZE) + j;
    }
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according to page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int *avail) {
  STATWORD ps;
  disable(ps);

  // if there is an unmapped frame, just use it
  for (int i = 0; i < NFRAMES; i++) {
    if (frm_tab[i].fr_status == FRM_UNMAPPED) {
      *avail = i;
      restore(ps);
      return OK;
    }
  }

  // ask the replacement policy for a victim
  int victim_frame_idx = rp_pop_victim();
  if (victim_frame_idx == SYSERR) {
    FRAME_DEBUG_PRINTF("get_frm: rp_pop_victim failed\n");
    restore(ps);
    return SYSERR;
  }

  // free the victim frame
  if (free_frm(victim_frame_idx) == SYSERR) {
    FRAME_DEBUG_PRINTF("get_frm: free_frm failed\n");
    restore(ps);
    return SYSERR;
  }

  // return the victim frame
  *avail = victim_frame_idx;
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * free_dir_frame - free a directory frame and its mapped frames
 *-------------------------------------------------------------------------
 */
static int free_dir_frame(int frame_id) {
  pd_t *page_directory = (pd_t *)((unsigned long)(FRAME0 + frame_id) * NBPG);
  for (int i = 0; i < PAGESIZE; i++) {
    if ((page_directory[i].pd_pres == TRUE) &&
        (page_directory[i].pd_global != TRUE)) {
      free_frm(page_directory[i].pd_base - FRAME0);
      rp_remove_frame(page_directory[i].pd_base - FRAME0);
    }
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * free_tbl_frame - free a table frame and its mapped frames
 *-------------------------------------------------------------------------
 */
static int free_tbl_frame(int frame_id) {
  pt_t *page_table = (pt_t *)frame_address(frame_id);
  for (int i = 0; i < PAGESIZE; i++) {
    if (page_table[i].pt_pres == TRUE) {
      int frm_id = page_table[i].pt_base - FRAME0;
      free_frm(frm_id);
      rp_remove_frame(frm_id);
    }
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * free_page_frame - free a page frame and write back to backing store
 *-------------------------------------------------------------------------
 */
static int free_page_frame(int frame_id) {
  unsigned char *page_frame = (unsigned char *)frame_address(frame_id);
  unsigned long vaddr = frm_tab[frame_id].fr_vpno * NBPG;
  int store, pageth;
  int pid = frm_tab[frame_id].fr_pid;
  register unsigned int pd_offset = ((virt_addr_t *)(&vaddr))->pd_offset;
  register unsigned int pt_offset = ((virt_addr_t *)(&vaddr))->pt_offset;
  register unsigned int pg_offset = ((virt_addr_t *)(&vaddr))->pg_offset;
  pd_t *page_directory = (pd_t *)(proctab[pid].pdbr);
  pt_t *page_table =
      (pt_t *)((unsigned long)(page_directory[pd_offset].pd_base * NBPG));

  // lookup the backing store and page index
  if (bsm_lookup(frm_tab[frame_id].fr_pid, vaddr, &store, &pageth) == SYSERR) {
    FRAME_DEBUG_PRINTF("free_page_frame: bsm_lookup failed [pid: %d] [vaddr: %d]\n", pid, vaddr);
    return SYSERR;
  }

  // write the page frame to the backing store
  write_bs((char *)(page_frame), store, pageth);

  // free the page table entry
  page_table[pt_offset].pt_pres = FALSE;
  page_table[pt_offset].pt_write = FALSE;
  page_table[pt_offset].pt_user = FALSE;
  page_table[pt_offset].pt_pwt = FALSE;
  page_table[pt_offset].pt_pcd = FALSE;
  page_table[pt_offset].pt_acc = FALSE;
  page_table[pt_offset].pt_mbz = FALSE;
  page_table[pt_offset].pt_dirty = FALSE;
  page_table[pt_offset].pt_global = FALSE;
  page_table[pt_offset].pt_avail = FALSE;
  page_table[pt_offset].pt_base = FALSE;

  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame and handle its type-specific cleanup
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int frame_id) {
  if (frame_id < 0 || frame_id >= NFRAMES) {
    FRAME_DEBUG_PRINTF(
        "free_frm: invalid frame id; id: %d, has to be between 0 and %d\n",
        frame_id, NFRAMES);
    return SYSERR;
  }

  STATWORD ps;
  disable(ps);

  if (frm_tab[frame_id].fr_status == FRM_UNMAPPED) {
    FRAME_DEBUG_PRINTF("free_frm: frame already unmapped; id: %d\n", frame_id);
    restore(ps);
    return SYSERR;
  }

  int status = OK;
  switch (frm_tab[frame_id].fr_type) {
  case FR_DIR:
    FRAME_DEBUG_PRINTF("free_frm: freeing directory frame; id: %d\n", frame_id);
    status = free_dir_frame(frame_id);
    break;
  case FR_TBL:
    FRAME_DEBUG_PRINTF("free_frm: freeing table frame; id: %d\n", frame_id);
    status = free_tbl_frame(frame_id);
    break;
  case FR_PAGE:
    FRAME_DEBUG_PRINTF("free_frm: freeing page frame; id: %d\n", frame_id);
    status = free_page_frame(frame_id);
    break;
  default:
    FRAME_DEBUG_PRINTF("free_frm: unknown frame type %d\n",
                       frm_tab[frame_id].fr_type);
    break;
  }

  if (status == SYSERR) {
    FRAME_DEBUG_PRINTF("free_frm: failed to free frame; id: %d\n", frame_id);
    restore(ps);
    return SYSERR;
  }

  frm_tab[frame_id].fr_status = FRM_UNMAPPED;
  frm_tab[frame_id].fr_pid = -1;
  frm_tab[frame_id].fr_vpno = -1;
  frm_tab[frame_id].fr_type = -1;
  frm_tab[frame_id].fr_dirty = FALSE;
  rp_remove_frame(frame_id);
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_page_directory - get a page directory frame for a process
 *-------------------------------------------------------------------------
 */
int get_page_directory(int pid) {
  STATWORD ps;
  disable(ps);

  int frm_id;
  pd_t *pd;
  if (get_frm(&frm_id) == SYSERR) {
    restore(ps);
    return SYSERR;
  }

  frm_tab[frm_id].fr_status = FRM_MAPPED;
  frm_tab[frm_id].fr_pid = pid;
  frm_tab[frm_id].fr_vpno = -1;
  frm_tab[frm_id].fr_type = FR_DIR;
  frm_tab[frm_id].fr_dirty = FALSE;

  proctab[pid].pdbr = frame_address(frm_id);
  for (int i = 0; i < PAGESIZE; i++) {
    pd = (pd_t *)(frame_address(frm_id) + (i * sizeof(pd_t)));
    switch (i) {
    case 0:
    case 1:
    case 2:
    case 3:
      pd->pd_pres = TRUE;
      pd->pd_write = TRUE;
      pd->pd_user = FALSE;
      pd->pd_pwt = FALSE;
      pd->pd_pcd = FALSE;
      pd->pd_acc = FALSE;
      pd->pd_mbz = FALSE;
      pd->pd_fmb = FALSE;
      pd->pd_global = TRUE;
      pd->pd_avail = FALSE;
      switch (i) {
      case 0:
        pd->pd_base = gpt.gpt1_idx;
        break;
      case 1:
        pd->pd_base = gpt.gpt2_idx;
        break;
      case 2:
        pd->pd_base = gpt.gpt3_idx;
        break;
      case 3:
        pd->pd_base = gpt.gpt4_idx;
        break;
      }
      break;
    default:
      pd->pd_pres = FALSE;
      pd->pd_write = FALSE;
      pd->pd_user = FALSE;
      pd->pd_pwt = FALSE;
      pd->pd_pcd = FALSE;
      pd->pd_acc = FALSE;
      pd->pd_mbz = FALSE;
      pd->pd_fmb = FALSE;
      pd->pd_global = FALSE;
      pd->pd_avail = FALSE;
      pd->pd_base = FALSE;
      break;
    }
  }
  restore(ps);
  return frm_id;
}

/*-------------------------------------------------------------------------
 * get_page_table - get a page table frame for a process
 *-------------------------------------------------------------------------
 */
int get_page_table(int pid) {
  STATWORD ps;
  disable(ps);

  int frm_id;
  pt_t *page_table;
  if (get_frm(&frm_id) == SYSERR) {
    FRAME_DEBUG_PRINTF("get_page_table: get_frm failed\n");
    restore(ps);
    return SYSERR;
  }

  frm_tab[frm_id].fr_status = FRM_MAPPED;
  frm_tab[frm_id].fr_pid = pid;
  frm_tab[frm_id].fr_vpno = -1;
  frm_tab[frm_id].fr_type = FR_TBL;
  frm_tab[frm_id].fr_dirty = FALSE;

  for (int i = 0; i < PAGESIZE; i++) {
    page_table = (pt_t *)(frame_address(frm_id) + (i * sizeof(pt_t)));
    page_table->pt_pres = FALSE;
    page_table->pt_write = FALSE;
    page_table->pt_user = FALSE;
    page_table->pt_pwt = FALSE;
    page_table->pt_pcd = FALSE;
    page_table->pt_acc = FALSE;
    page_table->pt_mbz = FALSE;
    page_table->pt_dirty = FALSE;
    page_table->pt_global = FALSE;
    page_table->pt_avail = FALSE;
    page_table->pt_base = FALSE;
  }
  restore(ps);
  return frm_id;
}