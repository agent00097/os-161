/*
 * Scheduler.
 *
 * The default scheduler is very simple, just a round-robin run queue.
 * You'll want to improve it.
 */

#include <types.h>
#include <lib.h>
#include <scheduler.h>
#include <thread.h>
#include <machine/spl.h>
#include <queue.h>
#include <clock.h>

/*
 *  Scheduler data
 */
#ifdef IS_MLFQ
int scheduler_type = MLFQ;
#endif

#ifdef IS_RR
int scheduler_type = RR;
#endif

// get the clock interrupt frequency 
int hz = HZ;

// pool of numbers from which we pull randomly - better chance to get pri 1
int numbers[] = {1, 1, 1, 2, 2};

// Queue of runnable threads
static struct queue *runqueue; // pri1
static struct queue *rq2; // pri2

/*
 * Setup function
 */
void
scheduler_bootstrap(void)
{
	runqueue = q_create(32);
	if (runqueue == NULL) {
		panic("scheduler: Could not create run queue\n");
	}
	switch(scheduler_type) 
	{
		case RR:
			break;
		case MLFQ:
			rq2 = q_create(32);
			if (runqueue == NULL) {
				panic("scheduler: Could not create rq2\n");
			}
			break;
	}
}

/*
 * Ensure space for handling at least NTHREADS threads.
 * This is done only to ensure that make_runnable() does not fail -
 * if you change the scheduler to not require space outside the
 * thread structure, for instance, this function can reasonably
 * do nothing.
 */
int
scheduler_preallocate(int nthreads)
{
	int initial_result;
	int result; 
    assert(curspl>0);
	switch(scheduler_type) {
		case RR:
			result = q_preallocate(runqueue, nthreads);
			break;
		case MLFQ:
			initial_result = q_preallocate(runqueue, nthreads);
			result = initial_result & (q_preallocate(rq2, nthreads));
			break;
	}
	return result;
}

/*
 * This is called during panic shutdown to dispose of threads other
 * than the one invoking panic. We drop them on the floor instead of
 * cleaning them up properly; since we're about to go down it doesn't
 * really matter, and freeing everything might cause further panics.
 */
void
scheduler_killall(void)
{
    assert(curspl>0);
    while (!q_empty(runqueue)) {
        struct thread *t = q_remhead(runqueue);
        kprintf("scheduler: Dropping thread %s.\n", t->t_name);
    }
	// additional work if MLFQ
	if (scheduler_type == MLFQ) 
	{
		while (!q_empty(rq2)) {
			struct thread *t = q_remhead(rq2);
			kprintf("scheduler: Dropping thread %s.\n", t->t_name);
		}
	}
}

/*
 * Cleanup function.
 *
 * The queue objects to being destroyed if it's got stuff in it.
 * Use scheduler_killall to make sure this is the case. During
 * ordinary shutdown, normally it should be.
 */
void
scheduler_shutdown(void)
{
    scheduler_killall();

    assert(curspl>0);
    q_destroy(runqueue);
    runqueue = NULL;

	// additional work if MLFQ
	if (scheduler_type == MLFQ) {
		q_destroy(rq2);
		rq2 = NULL;
	}
}

/*
 * Actual scheduler. Returns the next thread to run.  Calls cpu_idle()
 * if there's nothing ready. (Note: cpu_idle must be called in a loop
 * until something's ready - it doesn't know whether the things that
 * wake it up are going to make a thread runnable or not.)
 */
struct thread *
scheduler(void)
{
    // meant to be called with interrupts off
    assert(curspl>0);

	//print_run_queue();
	
	// Quantums for the 2 different queueus
	switch(scheduler_type) {
		case RR:
			while (q_empty(runqueue)) {
				cpu_idle();
			}
			quantum = hz;
			return q_remhead(runqueue);
		case MLFQ:
			while(q_empty(runqueue) && q_empty(rq2)) {
				cpu_idle();
			}
			
			// pick a queue to pull from, favoring higher priority.
			int index = random() % 4;
			int q = numbers[index];

			switch(q) {
			
				case PRI_1:
					
					// if there's pri 1 thread, run that
					if(!(q_empty(runqueue))) {
						quantum = hz;
						return q_remhead(runqueue);
					}

					// else, run a pri 2 thread
					if (!(q_empty(rq2))) {
						quantum = hz * 2;
						return q_remhead(rq2);
					}
					break;	
				case PRI_2:
					
					// if q is pri 2, try run a pri 2 thread
					if(!(q_empty(rq2))) {
						quantum = hz * 2;
						return q_remhead(rq2);
					}
					// else, try to run a pri 1 thread
					if(!(q_empty(runqueue))) {
						quantum = hz;
						return q_remhead(runqueue);
					}
					break;
			}
	}
	// if we fell through without returning, we couldn't schedule
	return NULL;
}

/*
 * Make a thread runnable.
 * With the base scheduler, just add it to the end of the run queue.
 */
int
make_runnable(struct thread *t)
{
    // meant to be called with interrupts off
    assert(curspl>0);
	switch(scheduler_type) 
	{
    	case RR: return q_addtail(runqueue, t);
		case MLFQ:
			switch(t->priority) {
				case PRI_1: return q_addtail(runqueue, t);
				case PRI_2: return q_addtail(rq2, t);
			}
	}
	// if we fell through, return 0
	return 0;
}

/*
 * Debugging function to dump the run queue.
 */
void
print_run_queue(void)
{
    /* Turn interrupts off so the whole list prints atomically. */
    int spl = splhigh();

    int i,k=0;
    i = q_getstart(runqueue);

    while (i!=q_getend(runqueue)) {
        struct thread *t = q_getguy(runqueue, i);
        kprintf(" runqueue %2d: %s %p %d\n", k, t->t_name, t->t_sleepaddr, t->priority);
        i=(i+1)%q_getsize(runqueue);
        k++;
    }

    int l,m=0;
    l = q_getstart(rq2);
    while (l!=q_getend(runqueue)) {
        struct thread *t = q_getguy(runqueue, l);
        kprintf(" rq2 %2d: %s %p %d\n", m, t->t_name, t->t_sleepaddr, t->priority);
        l=(l+1)%q_getsize(rq2);
        m++;
    }

    splx(spl);
}


