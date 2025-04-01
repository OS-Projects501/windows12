/* vcreate.c - vcreate */

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

SYSCALL vcreate(procaddr, ssize, hsize, priority, name, nargs, args)
int *procaddr; /* process address		*/
int ssize;     /* stack size in words		*/
int hsize;     /* heap size in pages		*/
int priority;  /* process priority > 0, skipping NULL priority	*/
char *name;    /* name (for debugging)		*/
int nargs;     /* number of args that follow	*/
long args;     /* arguments (treated like an array)*/
{
    STATWORD ps;
    int pid;
    struct pentry *pptr;
    bsd_t bs;

    disable(ps);

    /* Working on Argument validation */
    if (ssize < MINSTK || priority < 1 || hsize <= 0 || hsize > MAX_HPAGES)
    {
        restore(ps);
        return SYSERR;
    }

    /* creating the process using original create() */
    pid = create(procaddr, ssize, priority, name, nargs, args);
    if (pid == SYSERR)
    {
        restore(ps);
        return SYSERR;
    }

    /* Get a backing store for the process */
    bs = get_bs(pid, hsize);
    if (bs == SYSERR)
    {
        kill(pid);
        restore(ps);
        return SYSERR;
    }

    /* Set up the process's virtual heap */
    pptr = &proctab[pid];
    pptr->store = bs;
    pptr->vhpno = FIRST_VPAGE; /* Virtual heap starts at this page */
    pptr->vhpnpages = hsize;
    pptr->vmemlist = NULL; /* Will be initialized on first vgetmem call */

    /* Map the backing store to virtual memory */
    if (xmmap(pptr->vhpno, bs, hsize) == SYSERR)
    {
        release_bs(bs);
        kill(pid);
        restore(ps);
        return SYSERR;
    }

    restore(ps);
    return pid;
}