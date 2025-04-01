/* vgetmem.c - vgetmem */
#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

WORD *vgetmem(nbytes)
unsigned nbytes;
{
    STATWORD ps;
    struct mblock *prev, *curr, *leftover;
    struct pentry *pptr;
    unsigned vheapstart;

    disable(ps);

    /* Checking  if  there are valid nbytes */
    if (nbytes == 0)
    {
        restore(ps);
        return (WORD *)SYSERR;
    }

    /* process entry */
    pptr = &proctab[currpid];

    /* Checking if the process has a virtual Heap */
    if (pptr->vhpnpages <= 0)
    {
        kprintf("vgetmem: process %d has no virtual heap\n", currpid);
        restore(ps);
        return (WORD *)SYSERR;
    }

    /* Calculating the base address of the virtual heap */
    vheapstart = pptr->vhpno * NBPG;

    /* Initializing virtual memory list if it's first call */
    if (pptr->vmemlist == NULL)
    {
        pptr->vmemlist = (struct mblock *)vheapstart;
        pptr->vmemlist->mnext = (struct mblock *)vheapstart;
        pptr->vmemlist->mlen = pptr->vhpnpages * NBPG;
    }

    /* Round request size up to a multiple of MBLOCK  */
    nbytes = (unsigned int)roundmb(nbytes);

    /* Allocate memory from the virtual heap list */
    prev = pptr->vmemlist;
    curr = prev->mnext;

    while (curr != pptr->vmemlist)
    {
        if (curr->mlen >= nbytes)
        {
            /* If  Found a block big enough */
            leftover = (struct mblock *)((unsigned)curr + nbytes);

            /* Updating the memory list (not functional, requires addition ) */
            prev->mnext = leftover;
            leftover->mnext = curr->mnext;
            leftover->mlen = curr->mlen - nbytes;

            restore(ps);
            return (WORD *)curr;
        }

        prev = curr;
        curr = curr->mnext;
    }

    /* If no memory block big enough */
    restore(ps);
    return (WORD *)SYSERR;
}