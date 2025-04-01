/* xmmap.c - xmmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * xmmap - map a device (or file) into a process's address space
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(virtpage, source, npages)
int virtpage; /* Starting virtual page number to map */
bsd_t source; /* Backing store ID */
int npages;   /* Number of pages to map */
{
    STATWORD ps;
    bs_map_t *bs_map;

    /* Basic parameter validation */
    if (virtpage < FIRST_VPAGE || source < 0 || source >= NBS || npages <= 0)
    {
        return SYSERR;
    }

    disable(ps);

    /* Get the backing store map */
    bs_map = &bsm_tab[source];

    /* Check if backing store is available */
    if (bs_map->bs_status == BS_UNMAPPED)
    {
        kprintf("xmmap: backing store %d is not allocated\n", source);
        restore(ps);
        return SYSERR;
    }

    /* Verify that process is allowed to map this backing store */
    if (bs_map->bs_pid != currpid && bs_map->bs_pid != -1)
    {
        kprintf("xmmap: backing store %d is already mapped by another process\n", source);
        restore(ps);
        return SYSERR;
    }

    /* Verify that the number of pages requested is valid */
    if (npages > bs_map->bs_npages)
    {
        kprintf("xmmap: requested %d pages, but backing store only has %d pages\n",
                npages, bs_map->bs_npages);
        restore(ps);
        return SYSERR;
    }

    /* Map the backing store to virtual memory */
    bs_map->bs_pid = currpid;
    bs_map->bs_vpno = virtpage;
    bs_map->bs_status = BS_MAPPED;

    /* A page fault will occur when the process accesses this memory */
    /* The page fault handler will bring in the page as needed */

    restore(ps);
    return OK;
}