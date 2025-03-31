#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <paging.h>

SYSCALL read_bs(char *dst, bsd_t bs_id, int page) {

  /* fetch page page from map map_id
     and write beginning at dst.
  */
   if (bs_id < 0 || bs_id >= NBS) {
        return SYSERR; 
    }

   if (bsm_tab[bs_id].bs_status != BSM_MAPPED ||
        page < 0 || page >= bsm_tab[bs_id].bs_npages) {
       
        return SYSERR;
    }
   void * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
   bcopy(phy_addr, (void*)dst, NBPG);

   return OK;
}


