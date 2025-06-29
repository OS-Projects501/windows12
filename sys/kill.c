/* kill.c - kill */

#include <conf.h>
#include <io.h>
#include <kernel.h>
#include <mem.h>
#include <paging.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid) {
  STATWORD ps;
  struct pentry *pptr; /* points to proc. table for pid*/
  int dev;

  disable(ps);
  if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
    restore(ps);
    return (SYSERR);
  }
  if (--numproc == 0)
    xdone();

  dev = pptr->pdevs[0];
  if (!isbaddev(dev))
    close(dev);
  dev = pptr->pdevs[1];
  if (!isbaddev(dev))
    close(dev);
  dev = pptr->ppagedev;
  if (!isbaddev(dev))
    close(dev);

  send(pptr->pnxtkin, pid);
  freestk(pptr->pbase, pptr->pstklen);
  switch (pptr->pstate) {

  case PRCURR:
    pptr->pstate = PRFREE; /* suicide */
    resched();

  case PRWAIT:
    semaph[pptr->psem].semcnt++;

  case PRREADY:
    dequeue(pid);
    pptr->pstate = PRFREE;
    break;

  case PRSLEEP:
  case PRTRECV:
    unsleep(pid);
    /* fall through	*/
  default:
    pptr->pstate = PRFREE;
  }

  // relase backing stre
  release_bs(pptr->store);
  free_frm((pptr->pdbr) / NBPG - FRAME0); // free the page directory

  restore(ps);
  return (OK);
}
