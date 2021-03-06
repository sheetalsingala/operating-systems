/* releaseall.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <sem.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * releaseall.c  --  calling process releases all locks
 *------------------------------------------------------------------------
 */

int releaseall(int nlocks, long locks)
{
    //struct	pentry	*pptr;
    int pptr;
    struct	lentry	*lptr;
    STATWORD ps; 
    unsigned long *lock = (unsigned long *)(&locks);
    int i;
    int p0, p1;
    int flag1 = 1;
    int flag=1;
    disable(ps);
  //  kprintf("\nreleaseall.c");
    for(i=0;i<nlocks;i++)          /*Parse the entire list of locks */
    {   
        lptr= &ltable[(*lock)%NLOCKS];
        if(proctab[currpid].lacquired[(*lock)%NLOCKS] == 0 || isbadlock((*lock)%NLOCKS) || (lptr->lstate==LFREE )){ /* Return SYSERR if lock is invalid*/  
            flag = 0;
        }
        p1 = lptr->ltail;
//	kprintf("\nQueue empty? %d", q[q[p1].qprev].qkey);
        if(q[p1].qprev == lptr->lhead){     /*Wait queue is empty */
//		kprintf("\nQueue empty");
                lptr->lcnt+=1;
                lptr->lholdprocs[currpid] = 0;
                proctab[currpid].lacquired[(*lock)%NLOCKS] = 0;
                lptr->ltype = LINIT;
        }
       
        if(ltable[(*lock)%NLOCKS].ltype==WRITE){             /* Lock type is write, remove first waiting proc-> if its a reader*/
  	  //                            kprintf("\n Lock type is WRITE");                  /* Remove all waiting readers higher than highest waiting writer*/ 
	int l0 = *lock; 
	flag1 = 0; 
	//	kprintf("\n releaseall WRITE");                   
           if(q[maxReader(l0)].qkey == q[maxWriter(l0)].qkey ) 
                { //kprintf("\smae lock prio");
                   // proctab[maxReader(l0)].plwaittime  = ctr1000 - proctab[maxReader(l0)].plwaittime;
                   // proctab[maxWriter(l0)].plwaittime  = ctr1000 - proctab[maxWriter(l0)].plwaittime;
	//	    kprintf("\n Waiting times  Reader = %d and Writer %d",proctab[maxReader(l0)].plwaittime, proctab[maxWriter(l0)].plwaittime);
		    if(ctr1000 - proctab[maxWriter(l0)].plwaittime <= ctr1000 - proctab[maxReader(l0)].plwaittime+lockPolicy){
                    while(ctr1000 - proctab[maxWriter(l0)].plwaittime <= ctr1000 - proctab[maxReader(l0)].plwaittime + lockPolicy && q[maxReader(l0)].qkey == q[maxWriter(l0)].qkey){
                        /* remove reader from queue */
                        /* Does this loop end */
	                
			int k = maxReader(*lock);
                        ready(dequeue(k), RESCHNO);
                        ltable[(*lock)%NLOCKS].ltype = READ;
                        ltable[(*lock)%NLOCKS].numreaders +=1;
                        proctab[k].lacquired[(*lock)%NLOCKS] = 1;      
                        ltable[(*lock)%NLOCKS].lholdprocs[k] = 1;
			proctab[k].plock = -1;
			proctab[k].plwaitret = OK;  
                    }
			}
			else{	
				int l = maxWriter(*lock);
				ready(dequeue(l), RESCHNO);
				ltable[(*lock)%NLOCKS].ltype = WRITE;
				ltable[(*lock)%NLOCKS].lholdprocs[l] = 1;
				proctab[l].plock = -1;
				proctab[l].lacquired[(*lock)%NLOCKS] = 1;
				proctab[l].plwaitret =OK;		
                }}
                else{                                   /* Condition when prio is not same */
                                                        /* Dequeue first proc and decide */
                        pptr = getlast(lptr->ltail);
                        if(proctab[pptr].pltype == READ){       /* If dequeued proc is reader */
                        ready(pptr, RESCHNO);
                        ltable[(*lock)%NLOCKS].ltype = READ;
                        ltable[(*lock)%NLOCKS].numreaders +=1;
                        proctab[pptr].lacquired[(*lock)%NLOCKS] = 1;      
                        ltable[(*lock)%NLOCKS].lholdprocs[pptr] = 1; 
			proctab[pptr].plock = -1;
			proctab[pptr].plwaitret = OK;
                        p0 = q[lptr->ltail].qprev;
		//	kprintf("\nShould enter here %d", proctab[p0].pprio);
                        while(q[p0].qprev != NULL || proctab[p0].pltype == WRITE){ /* Reach end of list or first writer */
				//kprintf("Should not enter here!");
				int temp = getlast(lptr->ltail);
                                ready(temp, RESCHNO);
                                ltable[(*lock)%NLOCKS].numreaders +=1 ;
                                proctab[temp].lacquired[(*lock)%NLOCKS] = 1;      
                                ltable[(*lock)%NLOCKS].lholdprocs[temp] = 1;
				proctab[temp].plock = -1;
				proctab[temp].plwaitret = OK; 
                              	p0 = q[p0].qprev;  
                                }
		  		int lo =*lock;
                                if(q[maxReader(lo)].qkey == q[maxWriter(lo)].qkey && q[maxReader(lo)].qkey != MININT) 
                {//	kprintf("Here?");
                    //proctab[maxReader(lo)].plwaittime  = ctr1000 - proctab[maxReader(lo)].plwaittime;
                    //proctab[maxWriter(lo)].plwaittime  = ctr1000 - proctab[maxWriter(lo)].plwaittime;
		    if(ctr1000 - proctab[maxWriter(lo)].plwaittime <= ctr1000 - proctab[maxReader(lo)].plwaittime + lockPolicy){
                    while(ctr1000 - proctab[maxWriter(lo)].plwaittime <= ctr1000 - proctab[maxReader(lo)].plwaittime + lockPolicy  && q[maxReader(lo)].qkey == q[maxWriter(lo)].qkey){
                        /* remove reader from queue */
                        /* Does this loop end */
			int temp = maxReader(*lock);
                        ready(dequeue(temp), RESCHNO);
                        ltable[(*lock)%NLOCKS].ltype = READ;
                        ltable[(*lock)%NLOCKS].numreaders +=1;
                        proctab[temp].lacquired[(*lock)%NLOCKS] = 1;      
                        ltable[(*lock)%NLOCKS].lholdprocs[temp] = 1;
			proctab[temp].plock = -1;
			proctab[temp].plwaitret = OK;  
                    }}}
                            }
                        else{                           /* If dequeued proc is writer */
                            ready(pptr, RESCHNO);
                            ltable[(*lock)%NLOCKS].ltype = WRITE;
                            proctab[pptr].lacquired[(*lock)%NLOCKS] = 1;      
                            ltable[(*lock)%NLOCKS].lholdprocs[pptr] = 1;
			    proctab[pptr].plock = -1;
			    proctab[pptr].plwaitret = OK;

                        }
                    
            }
        }
        if(ltable[(*lock)%NLOCKS].ltype==READ && flag1 == 1){
	    //kprintf("Num Readers actual %d ", ltable[*lock].numreaders);
            ltable[(*lock)%NLOCKS].numreaders -= 1; 
            if(ltable[(*lock)%NLOCKS].numreaders > 0){       /* If number of readers is not zero */
                lptr->lholdprocs[currpid] = 0;
                proctab[currpid].lacquired[(*lock)%NLOCKS] = 0;
//		kprintf("Change in readers %d", lptr->numreaders);
            }
            if(ltable[(*lock)%NLOCKS].numreaders  == 0){
	//	kprintf("No more readers");             /* Lock type is read and number of readers is zero */
		int lo = *lock;
	//	kprintf("MaxWriter = %d", q[maxWriter(lo)].qkey);
	//	kprintf("MaxReader = %d", q[maxReader(lo)].qkey);
                if(q[maxReader(lo)].qkey == q[maxWriter(lo)].qkey ) 
                {//	kprintf("Here?");
                   // proctab[maxReader(lo)].plwaittime  = ctr1000 - proctab[maxReader(lo)].plwaittime;
                   // proctab[maxWriter(lo)].plwaittime  = ctr1000 - proctab[maxWriter(lo)].plwaittime;
		    if(ctr1000 - proctab[maxWriter(lo)].plwaittime <= ctr1000 - proctab[maxReader(lo)].plwaittime + lockPolicy){
                    while(ctr1000 - proctab[maxWriter(lo)].plwaittime <= ctr1000 - proctab[maxReader(lo)].plwaittime + lockPolicy  && q[maxReader(lo)].qkey == q[maxWriter(lo)].qkey){
                        /* remove reader from queue */
                        /* Does this loop end */
			int temp = maxReader(*lock);
                        ready(dequeue(temp), RESCHNO);
                        ltable[(*lock)%NLOCKS].ltype = READ;
                        ltable[(*lock)%NLOCKS].numreaders +=1;
                        proctab[temp].lacquired[(*lock)%NLOCKS] = 1;      
                        ltable[(*lock)%NLOCKS].lholdprocs[temp] = 1;
			proctab[temp].plock = -1;
			proctab[temp].plwaitret = OK;  
                    }}
			else{
			int wr = maxWriter(*lock);
			ready(dequeue(wr), RESCHNO);
			ltable[(*lock)%NLOCKS].ltype = WRITE;
			proctab[wr].lacquired[(*lock)%NLOCKS] = 1;
			proctab[wr].plock = -1;
			proctab[wr].plwaitret = OK;
			ltable[(*lock)%NLOCKS].lholdprocs[wr] = 1;
					
                }}
	
                else{                                   /* Condition when prio is not same */
          //      	kprintf("Should have entered here!");
		//	kprintf("Laist element %d prio %d", q[lptr->ltail].qprev, q[q[lptr->ltail].qprev].qkey);                                        /* Dequeue first proc and decide */
                        pptr = getlast(lptr->ltail);
	//		kprintf( "get first %d", pptr);
                        if(proctab[pptr].pltype == READ){       /* If dequeued proc is reader */
		//	kprintf("Reader has accessed");
                        ready(pptr, RESCHNO);
                        ltable[(*lock)%NLOCKS].ltype = READ;
                        ltable[(*lock)%NLOCKS].numreaders +=1;
                        proctab[pptr].lacquired[(*lock)%NLOCKS] = 1;      
                        ltable[(*lock)%NLOCKS].lholdprocs[pptr] = 1;
			proctab[pptr].plock = -1;
			proctab[pptr].plwaitret = OK; 
                        p0 = q[lptr->ltail].qprev;
                        while(q[q[p0].qprev].qprev != NULL || proctab[p0].pltype == WRITE){ /* Reach end of list or first writer */
				int tem = getlast(lptr->lhead);
                                ready(tem, RESCHNO);
                                ltable[(*lock)%NLOCKS].numreaders +=1 ;
                                proctab[tem].lacquired[(*lock)%NLOCKS] = 1;      
                                ltable[(*lock)%NLOCKS].lholdprocs[tem] = 1;
				proctab[tem].plock = -1;
				proctab[tem].plwaitret = OK; 
                                p0 = q[p0].qprev;  
                                }
				 int lo = *lock;
                                 if(q[maxReader(lo)].qkey == q[maxWriter(lo)].qkey && q[maxReader(lo)].qkey != MININT) 
                {//	kprintf("Here?");
                   // proctab[maxReader(lo)].plwaittime  = ctr1000 - proctab[maxReader(lo)].plwaittime;
                   // proctab[maxWriter(lo)].plwaittime  = ctr1000 - proctab[maxWriter(lo)].plwaittime;
		    if(ctr1000 - proctab[maxWriter(lo)].plwaittime <= ctr1000 - proctab[maxReader(lo)].plwaittime + lockPolicy){
                    while(ctr1000 - proctab[maxWriter(lo)].plwaittime <= ctr1000 - proctab[maxReader(lo)].plwaittime + lockPolicy  && q[maxReader(lo)].qkey == q[maxWriter(lo)].qkey){
                        /* remove reader from queue */
                        /* Does this loop end */
			int temp = maxReader(*lock);
                        ready(dequeue(temp), RESCHNO);
                        ltable[(*lock)%NLOCKS].ltype = READ;
                        ltable[(*lock)%NLOCKS].numreaders +=1;
                        proctab[temp].lacquired[(*lock)%NLOCKS] = 1;      
                        ltable[(*lock)%NLOCKS].lholdprocs[temp] = 1;
			proctab[temp].plock = -1;
			proctab[temp].plwaitret = OK;  
                    }}}
                            }
                        if(proctab[pptr].pltype == WRITE){                           /* If dequeued proc is writer */
	//		    kprintf("Writer %d has %s accessed", proctab[pptr].pltype, proctab[pptr].pname);
                            ready(pptr, RESCHNO);
                            ltable[(*lock)%NLOCKS].ltype = WRITE;
                            proctab[pptr].lacquired[(*lock)%NLOCKS] = 1;      
                            ltable[(*lock)%NLOCKS].lholdprocs[pptr] = 1;
			    proctab[pptr].plock = -1;
			    proctab[pptr].plwaitret = OK;
                        }
                    
            }
            }
            }
		updatePrio(*lock);
		proctab[currpid].lacquired[(*lock)%NLOCKS] = 0;
		ltable[(*lock)%NLOCKS].lholdprocs[currpid] = 0;
            
      }         

    if(flag == 0){
        restore(ps);
        return(SYSERR);
    }
   restore(ps);
   return(OK);
}


