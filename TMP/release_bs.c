#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
    //kprintf("To be implemented!\n");

    STATWORD ps;
	  disable(ps);


    //releases the backing store with the ID store.
    if (bs_id < 0 || bs_id >= NBS) {  
        return SYSERR;
    }

    if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED) {
        return OK;  
    }

    //kprintf("releasing\n");
    bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
    bsm_tab[bs_id].bs_npages = 0;
    bsm_tab[bs_id].bs_vpno = 0;
    bsm_tab[bs_id].bs_pid = -1;

  restore(ps);
   return OK;

}

