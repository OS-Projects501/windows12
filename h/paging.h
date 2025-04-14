#pragma once

#include "kernel.h"

typedef unsigned int bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres : 1;   /* page table present?		*/
  unsigned int pd_write : 1;  /* page is writable?		*/
  unsigned int pd_user : 1;   /* is use level protection?	*/
  unsigned int pd_pwt : 1;    /* write through cachine for pt?*/
  unsigned int pd_pcd : 1;    /* cache disable for this pt?	*/
  unsigned int pd_acc : 1;    /* page table was accessed?	*/
  unsigned int pd_mbz : 1;    /* must be zero			*/
  unsigned int pd_fmb : 1;    /* four MB pages?		*/
  unsigned int pd_global : 1; /* global (ignored)		*/
  unsigned int pd_avail : 3;  /* for programmer's use		*/
  unsigned int pd_base : 20;  /* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres : 1;   /* page is present?		*/
  unsigned int pt_write : 1;  /* page is writable?		*/
  unsigned int pt_user : 1;   /* is use level protection?	*/
  unsigned int pt_pwt : 1;    /* write through for this page? */
  unsigned int pt_pcd : 1;    /* cache disable for this page? */
  unsigned int pt_acc : 1;    /* page was accessed?		*/
  unsigned int pt_dirty : 1;  /* page was written?		*/
  unsigned int pt_mbz : 1;    /* must be zero			*/
  unsigned int pt_global : 1; /* should be zero in 586	*/
  unsigned int pt_avail : 3;  /* for programmer's use		*/
  unsigned int pt_base : 20;  /* location of page?		*/
} pt_t;

typedef struct {
  unsigned int pg_offset : 12; /* page offset			*/
  unsigned int pt_offset : 10; /* page table offset		*/
  unsigned int pd_offset : 10; /* page directory offset	*/
} virt_addr_t;

typedef struct {
  int bs_status; /* MAPPED or UNMAPPED		*/
  int bs_pid;    /* process id using this slot   */
  int bs_vpno;   /* starting virtual page number */
  int bs_npages; /* number of pages in the store */
  int bs_sem;    /* semaphore mechanism ?	*/
  int bs_private;
} bs_map_t;

typedef struct {
  int fr_status; /* MAPPED or UNMAPPED		*/
  int fr_pid;    /* process id using this frame  */
  int fr_vpno;   /* corresponding virtual page no*/
  int fr_refcnt; /* reference count		*/
  int fr_type;   /* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;
} fr_map_t;

typedef struct {
  int gpt1_idx; /* frame index of first global page table */
  int gpt2_idx; /* frame index of second global page table */
  int gpt3_idx; /* frame index of third global page table */
  int gpt4_idx; /* frame index of fourth global page table */
} global_page_table_t;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

#define NBPG 4096                 /* number of bytes per page	*/
#define FRAME0 1024               /* zero-th frame		*/
#define NFRAMES 23                /* number of frames		*/
#define N_BS 16                   /* number of backing store maps */
#define N_BS_FRAMES (2048 / N_BS) /* number of frames per backing store */
#define PAGESIZE 1024             /* number of fields in direcotry or page */
#define TOP_FRAME 4096

#define BSM_UNMAPPED 0
#define BSM_MAPPED 1

#define FRM_UNMAPPED 0
#define FRM_MAPPED 1

#define FR_PAGE 0
#define FR_TBL 1
#define FR_DIR 2

#define SC 1
#define FIFO 2

#define BACKING_STORE_BASE 0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00100000

extern global_page_table_t gpt;
int rp_push_frame(int frm_id);
int rp_pop_victim();
int rp_remove_frame(int frm_id);
int init_global_page_table();
int get_page_table(int pid);
int get_page_directory(int pid);
SYSCALL vfreemem(struct mblock *, unsigned int);
WORD *vgetmem(unsigned int);
SYSCALL free_frm(int id);
SYSCALL get_frm(int *avail);
SYSCALL init_frm();
SYSCALL init_bsm();
SYSCALL get_bsm(int *avail);
SYSCALL free_bsm(int i);
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int *store, int *pageth);
SYSCALL bsm_map(int pid, int vpno, int source, int npages);
SYSCALL bsm_unmap(int pid, int vpno, int flag);
SYSCALL srpolicy(int policy);

inline unsigned long frame_address(int frame_id) {
  return (unsigned long)((frame_id + FRAME0) * NBPG);
}

inline unsigned long backing_store_address(int bs_id) {
  return (unsigned long)(bs_id * BACKING_STORE_UNIT_SIZE + BACKING_STORE_BASE);
}