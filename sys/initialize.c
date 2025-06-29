/* initialize.c - nulluser, sizmem, sysinit */

#include <conf.h>
#include <i386.h>
#include <io.h>
#include <kernel.h>
#include <mem.h>
#include <paging.h>
#include <paging_fifo.h>
#include <paging_sc.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <sleep.h>
#include <tty.h>

#define INIT_DEBUG (1 & GLOBAL_DEBUG)

#if INIT_DEBUG
#include <stdio.h>
#define INIT_DEBUG_PRINTF(...)                                                 \
  do {                                                                         \
    kprintf("\033[32m[DEBUG] | [INIT] | ");                                    \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\033[0m");                                                        \
  } while (0)
#else
#define INIT_DEBUG_PRINTF(...)                                                 \
  do {                                                                         \
  } while (0)
#endif

/*#define DETAIL */
#define HOLESIZE (600)
#define HOLESTART (640 * 1024)
#define HOLEEND ((1024 + HOLESIZE) * 1024)
/* Extra 600 for bootp loading, and monitor */

extern int main(); /* address of user's main prog	*/

extern int start();

LOCAL sysinit();

/* Declarations of major kernel variables */
struct pentry proctab[NPROC]; /* process table			*/
int nextproc;                 /* next process slot to use in create	*/
struct sentry semaph[NSEM];   /* semaphore table			*/
int nextsem;                  /* next sempahore slot to use in screate*/
struct qent q[NQENT];         /* q table (see queue.c)		*/
int nextqueue;                /* next slot in q structure to use	*/
char *maxaddr;                /* max memory address (set by sizmem)	*/
struct mblock memlist;        /* list of free memory blocks		*/
#ifdef Ntty
struct tty tty[Ntty]; /* SLU buffers and mode control		*/
#endif

/* active system status */
int numproc;    /* number of live user processes	*/
int currpid;    /* id of currently running process	*/
int reboot = 0; /* non-zero after first boot		*/

int rdyhead, rdytail; /* head/tail of ready list (q indicies)	*/
char vers[80];
int console_dev; /* the console device			*/

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED, and      ***/
/***   must eventually be enabled explicitly.  This routine turns     ***/
/***   itself into the null process after initialization.  Because    ***/
/***   the null process must always remain ready to run, it cannot    ***/
/***   execute code that might cause it to be suspended, wait for a   ***/
/***   semaphore, or put to sleep, or exit.  In particular, it must   ***/
/***   not do I/O unless it uses kprintf for polled output.           ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  nulluser  -- initialize system and become the null process (id==0)
 *------------------------------------------------------------------------
 */
nulluser() /* babysit CPU when no one is home */
{
  int userpid;

  console_dev = SERIAL0; /* set console to COM0 */

  initevec();

  kprintf("system running up!\n");
  sysinit();

  enable(); /* enable interrupts */

  sprintf(vers, "PC Xinu %s", VERSION);
  kprintf("\n\n%s\n", vers);
  if (reboot++ < 1)
    kprintf("\n");
  else
    kprintf("   (reboot %d)\n", reboot);

  kprintf("%d bytes real mem\n", (unsigned long)maxaddr + 1);
#ifdef DETAIL
  kprintf("    %d", (unsigned long)0);
  kprintf(" to %d\n", (unsigned long)(maxaddr));
#endif

  kprintf("%d bytes Xinu code\n",
          (unsigned long)((unsigned long)&end - (unsigned long)start));
#ifdef DETAIL
  kprintf("    %d", (unsigned long)start);
  kprintf(" to %d\n", (unsigned long)&end);
#endif

#ifdef DETAIL
  kprintf("%d bytes user stack/heap space\n",
          (unsigned long)((unsigned long)maxaddr - (unsigned long)&end));
  kprintf("    %d", (unsigned long)&end);
  kprintf(" to %d\n", (unsigned long)maxaddr);
#endif

  kprintf("clock %sabled\n", clkruns == 1 ? "en" : "dis");

  /* create a process to execute the user's main program */
  userpid = create(main, INITSTK, INITPRIO, INITNAME, INITARGS);
  resume(userpid);

  while (TRUE)
    /* empty */;
}

static int pa03_init() {
  INIT_DEBUG_PRINTF("Initializing frame table\n");
  init_frm();

  INIT_DEBUG_PRINTF("Initializing backing store manager\n");
  init_bsm();

  INIT_DEBUG_PRINTF("Initializing page table\n");
  init_global_page_table();

  INIT_DEBUG_PRINTF("Initializing SC policy\n");
  sc_init();
  
  INIT_DEBUG_PRINTF("Initializing FIFO policy\n");
  fifo_init();
  
  INIT_DEBUG_PRINTF("Setting replacement policy to FIFO\n");
  srpolicy(FIFO);

  INIT_DEBUG_PRINTF("Getting page directory\n");
  get_page_directory(NULLPROC);

  return OK;
}

/*------------------------------------------------------------------------
 *  sysinit  --  initialize all Xinu data structeres and devices
 *------------------------------------------------------------------------
 */
LOCAL
sysinit() {
  static long currsp;
  int i, j;
  struct pentry *pptr;
  struct sentry *sptr;
  struct mblock *mptr;
  SYSCALL pfintr();

  numproc = 0; /* initialize system variables */
  nextproc = NPROC - 1;
  nextsem = NSEM - 1;
  nextqueue = NPROC; /* q[0..NPROC-1] are processes */

  /* initialize free memory list */
  /* PC version has to pre-allocate 640K-1024K "hole" */
  if (maxaddr + 1 > HOLESTART) {
    memlist.mnext = mptr = (struct mblock *)roundmb(&end);
    mptr->mnext = (struct mblock *)HOLEEND;
    mptr->mlen = (int)truncew(((unsigned)HOLESTART - (unsigned)&end));
    mptr->mlen -= 4;

    mptr = (struct mblock *)HOLEEND;
    mptr->mnext = 0;
    mptr->mlen = (int)truncew((unsigned)maxaddr - HOLEEND - NULLSTK);
    /*
                    mptr->mlen = (int) truncew((unsigned)maxaddr - (4096 - 1024
       ) *  4096 - HOLEEND - NULLSTK);
    */
  } else {
    /* initialize free memory list */
    memlist.mnext = mptr = (struct mblock *)roundmb(&end);
    mptr->mnext = 0;
    mptr->mlen = (int)truncew((unsigned)maxaddr - (int)&end - NULLSTK);
  }

  for (i = 0; i < NPROC; i++) /* initialize process table */
    proctab[i].pstate = PRFREE;

#ifdef MEMMARK
  _mkinit(); /* initialize memory marking */
#endif

#ifdef RTCLOCK
  clkinit(); /* initialize r.t.clock	*/
#endif

  mon_init(); /* init monitor */

#ifdef NDEVS
  for (i = 0; i < NDEVS; i++) {
    init_dev(i);
  }
#endif

  // initialize pa03-related 
  pa03_init();
  INIT_DEBUG_PRINTF("Setting up page table interrupt\n");
  set_evec(14, pfintr);
  INIT_DEBUG_PRINTF("Writing page directory to CR3\n");
  write_cr3(proctab[NULLPROC].pdbr);
  INIT_DEBUG_PRINTF("Enabling paging\n");
  enable_paging();

  pptr = &proctab[NULLPROC]; /* initialize null process entry */
  pptr->pstate = PRCURR;
  for (j = 0; j < 7; j++)
    pptr->pname[j] = "prnull"[j];
  pptr->plimit = (WORD)(maxaddr + 1) - NULLSTK;
  pptr->pbase = (WORD)maxaddr - 3;
  /*
          pptr->plimit = (WORD)(maxaddr + 1) - NULLSTK - (4096 - 1024 )*4096;
          pptr->pbase = (WORD) maxaddr - 3 - (4096-1024)*4096;
  */
  pptr->pesp = pptr->pbase - 4; /* for stkchk; rewritten before used */
  *((int *)pptr->pbase) = MAGIC;
  pptr->paddr = (WORD)nulluser;
  pptr->pargs = 0;
  pptr->pprio = 0;
  currpid = NULLPROC;

  for (i = 0; i < NSEM; i++) { /* initialize semaphores */
    (sptr = &semaph[i])->sstate = SFREE;
    sptr->sqtail = 1 + (sptr->sqhead = newqueue());
  }

  rdytail = 1 + (rdyhead = newqueue()); /* initialize ready list */

  return (OK);
}

stop(s) char *s;
{
  kprintf("%s\n", s);
  kprintf("looping... press reset\n");
  while (1)
    /* empty */;
}

delay(n) int n;
{ DELAY(n); }

#define NBPG 4096

/*------------------------------------------------------------------------
 * sizmem - return memory size (in pages)
 *------------------------------------------------------------------------
 */
long sizmem() {
  unsigned char *ptr, *start, stmp, tmp;
  int npages;

  /* at least now its hacked to return
     the right value for the Xinu lab backends (16 MB) */

  return 4096;

  start = ptr = 0;
  npages = 0;
  stmp = *start;
  while (1) {
    tmp = *ptr;
    *ptr = 0xA5;
    if (*ptr != 0xA5)
      break;
    *ptr = tmp;
    ++npages;
    ptr += NBPG;
    if ((int)ptr == HOLESTART) { /* skip I/O pages */
      npages += (1024 - 640) / 4;
      ptr = (unsigned char *)HOLEEND;
    }
  }
  return npages;
}
