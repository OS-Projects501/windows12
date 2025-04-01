/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

SYSCALL vfreemem(block, size)
struct mblock *block;
unsigned size;
{
    STATWORD ps;
    struct mblock *p, *q;
    struct pentry *pptr;
    unsigned top, vheapstart, vheapend;

    disable(ps);

    /* Get process entry */
    pptr = &proctab[currpid];

    /* Make sure process has a virtual heap and memory list is initialized */
    if (pptr->vhpnpages <= 0 || pptr->vmemlist == NULL)
    {
        restore(ps);
        return SYSERR;
    }

    /* Calculate virtual heap boundaries */
    vheapstart = pptr->vhpno * NBPG;
    vheapend = vheapstart + (pptr->vhpnpages * NBPG);

    /* Parameter validation */
    if (size == 0 || (unsigned)block < vheapstart ||
        (unsigned)block + size > vheapend)
    {
        restore(ps);
        return SYSERR;
    }

    /* Round up to a multiple of MBLOCK */
    size = (unsigned)roundmb(size);
    block = (struct mblock *)((unsigned)block);

    /* Find where the block fits in the free memory list */
    for (q = pptr->vmemlist, p = pptr->vmemlist->mnext;
         p != pptr->vmemlist && (unsigned)block > (unsigned)p;
         q = p, p = p->mnext)
    {
        /* Empty loop body - just finding the correct position */
    }

    /* Check if the block to free overlaps with any existing blocks */
    if (((unsigned)block + size) > (unsigned)p && p != pptr->vmemlist)
    {
        restore(ps);
        return SYSERR;
    }

    /* Try to coalesce with the next block if they're adjacent */
    top = (unsigned)block + size;
    if (top == (unsigned)p && p != pptr->vmemlist)
    {
        /* Merge with the next block */
        block->mlen = size + p->mlen;
        block->mnext = p->mnext;
    }
    else
    {
        /* Just insert the block */
        block->mlen = size;
        block->mnext = p;
    }

    /* Try to coalesce with the previous block if they're adjacent */
    if ((unsigned)q + q->mlen == (unsigned)block && q != pptr->vmemlist)
    {
        /* Merge with the previous block */
        q->mlen += block->mlen;
        q->mnext = block->mnext;
    }
    else
    {
        /* Just link the block in */
        q->mnext = block;
    }

    restore(ps);
    return OK;
}