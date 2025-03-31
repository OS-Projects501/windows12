#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

bs_map_t bsm_tab[NBS];

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
    //kprintf("To be implemented!\n");
    //kprintf("Entering get_bs.....\n");
    STATWORD ps;
	  disable(ps);
    if(bs_id<0 || bs_id>= NBS || npages==0 || npages>128){
      return SYSERR;
    }

    //a backing store with this ID already exists
    if (bsm_tab[bs_id].bs_status == BSM_MAPPED) {
        return bsm_tab[bs_id].bs_npages; 
    }
    //If a new backing store can be created
    bsm_tab[bs_id].bs_status = BSM_MAPPED;
    bsm_tab[bs_id].bs_npages = npages;
    bsm_tab[bs_id].bs_pid = -1;  
    bsm_tab[bs_id].bs_vpno = 0;  

    npages = bsm_tab[bs_id].bs_npages; 
    //kprintf("%d\n", npages); 

    restore(ps);
    
    return npages;

}


