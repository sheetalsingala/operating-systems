/* lock.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lock.c  --  acquirie a lock takes in lock id, lock type and prio
 *------------------------------------------------------------------------
 */
int lock (int ldes1, int type, int priority)
{
    STATWORD ps;    
	struct	lentry	*lptr;
	struct	pentry	*pptr;
    pptr = &proctab[currpid];
    lptr = &ltable[ldes1%NLOCKS];
   //kprintf("Lock! %d", currpid);
    disable(ps);
    
	
	if (isbadlock(ldes1%NLOCKS) || (lptr= &ltable[ldes1%NLOCKS])->lstate==LFREE || ldes1 != ldes1%NLOCKS + ltable[ldes1%NLOCKS].ver*NPROC ) {  /* Check if lock descriptor is valid*/
		restore(ps);
		return(SYSERR);
	}
	
    if(--(lptr->lcnt) < 0) 
    {//kprintf("Here!");
        if (lptr ->ltype == READ )
        {
            if(type == WRITE)           /* Requesting proc is a writer but resource already acquired by writer */
            {
		//kprintf("\n Type wait");
                pptr->pstate = PLWAIT;
                pptr->plock = ldes1;
                pptr->pltype = WRITE;
                pptr->plwaittime = ctr1000;
                insert(currpid,lptr->lhead,priority);
                pptr->plwaitret = OK;
		updatePrio(ldes1);
                resched();
                restore(ps);
                return pptr->plwaitret;      
            }
            else
            {	//kprintf("\n Type read");
		//kprintf("\n Writers prio %d",q[maxWriter(ldes1)].qkey);
	/*	if(priority == q[maxWriter(ldes1)].qkey && ctr1000 - proctab[maxWriter(ldes1)].plwaittime > 40)
		{
			pptr->pstate = PLWAIT;
			pptr->plock = ldes1;
			pptr->pltype = READ;
			pptr->plwaittime = ctr1000;
			insert(currpid, lptr->lhead, priority);
			pptr->plwaitret = OK;
			resched();
			restore(ps);
			return pptr->plwaitret;
		}*/
                if(priority > q[maxWriter(ldes1)].qkey ||( priority == q[maxWriter(ldes1)].qkey && ctr1000 - proctab[maxWriter(ldes1)].plwaittime <= lockPolicy)){        /* If priority of the process is lesser than highest waiting writer */
                    restore(ps);                        /* If equal to ? Check its waiting time */
                    lptr->numreaders += 1;
		    proctab[currpid].lacquired[ldes1%NLOCKS] = 1;
		    ltable[ldes1%NLOCKS].lholdprocs[currpid] = 1;
//		    kprintf("\nnum readers %d", lptr->numreaders);
                    return(OK);
                }
                else{
                    pptr->pstate = PLWAIT;
                    pptr->plock = ldes1;
                    pptr->pltype = READ;
                    pptr->plwaittime = ctr1000;
                    insert(currpid,lptr->lhead,priority);
                    pptr->plwaitret = OK;
		    updatePrio(ldes1);
                    resched();
                    restore(ps);
                    return pptr->plwaitret; 
                }
	}
}
        else            /* Already less than zero and writer has occupied lock, insert in wait queue */
        {
//	    kprintf("Writer type lock ");            
            pptr->pstate = PLWAIT;
            pptr->plock = ldes1;
            pptr->pltype = type;            /* Irrespective of whether it is reader or writer */
            pptr->plwaittime = ctr1000;
            insert(currpid,lptr->lhead,priority);
            pptr->plwaitret = OK;
	    //kprintf("\n \nLock.c plock %d", pptr->plock);
	    updatePrio(ldes1);
            resched();
            restore(ps);
            return pptr->plwaitret;
        }
        
    }
    
	restore(ps);
    lptr->ltype = type;
//kprintf("yaaaaa");
    if(type == READ){
        lptr->numreaders += 1;
    }
    proctab[currpid].lacquired[ldes1%NLOCKS] = 1;          /* Acquired lock */
    //kprintf("currpid %d", currpid);
    lptr->lholdprocs[currpid] = 1;                   /* Current prcoess is added to list of processes waiting on lock */
	return(OK);
}

int maxWriter(int ldes1)            /* Return the priority of highest waiting writer */
{
    struct	lentry	*lptr;
	int p0;
	lptr = &ltable[ldes1%NLOCKS];
//	kprintf("\n Max Writer");
    p0 = q[lptr->ltail].qprev;
   // kprintf("%d", proctab[p0].pltype);
    while(p0 != lptr->lhead){
        if(proctab[p0].pltype == WRITE){
	//	kprintf("Prio %d", q[p0].qkey);
            return(p0);
        }
	p0 = q[p0].qprev;
    }
    return(lptr->lhead);                    /* Return null if no writer */
}

int maxReader(int ldes1)            /* Return the priority of highest waiting reader */
{
    struct	lentry	*lptr;
    int p0;
    lptr = &ltable[ldes1%NLOCKS];
    p0 = q[lptr->ltail].qprev;
    while(p0 != lptr->lhead){
        if(proctab[p0].pltype == READ){
            return(p0);
        }
	p0 = q[p0].qprev;

    }
//kprintf("Head %d Prio %d", lptr->lhead, q[lptr->lhead].qkey);
    return(lptr->lhead);                    /* Return null if no reader */
}

