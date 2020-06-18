/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	// initialize the lock as available
	lock->flag = 0;

	// at init, no thread has the lock
	lock->holder = NULL;

	//kprintf("Lock created with the name of : %s\n", name);

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	// variable for priority level
	int spl;

	assert(lock != NULL);

	// disable interrupts
	spl = splhigh();

	// make sure we don't leave some threads permanently sleeping
	assert( thread_hassleepers(lock) ==0 );

	// return spl to previous value
	splx(spl);

	// free the lock
	kfree(lock->holder);
	kfree(lock->flag);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	
	// Disabiling the interrupts first
	int spl;
	spl = splhigh();
	
	assert(lock != NULL);
	assert(!lock_do_i_hold(lock));
	assert(in_interrupt == 0);

	while (lock->flag == 1)
	{ 
		// If lock is not availaible we must wait
		thread_sleep(lock);
	}

	// We can now take the lock!
	lock->flag = 1;
	lock->holder = curthread;

	// enabling all the interrupts again
	splx(spl);


}

void
lock_release(struct lock *lock)
{

	int spl;

	assert(lock !=NULL);
	assert( lock_do_i_hold(lock) );

	// disable interrupts
	spl = splhigh();
	
	// make the lock available
	lock->flag = 0; 
	lock->holder = NULL;
	
	//Wakeup the next thread waiting for the lock
	thread_wakeup(lock);

	//Re-enable all interrupts.
	splx(spl);

}

int
lock_do_i_hold(struct lock *lock)
{
	// disable interrupts
	int spl;
	spl = splhigh();

	int answer = (lock->holder == curthread);

	// resume interrupts
	splx(spl);

	// returns whether curthread is the lock holder
	return answer;
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	// add stuff here as needed
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	// add stuff here as needed
	
	kfree(cv->name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
// Go into kernel mode
	// Check that cv and lock are present
	int spl;

	// Check to make sure we have an actual lock
	assert(lock != NULL);

	// Check to make sure we have an actual cv
	assert(cv != NULL);

	spl = splhigh();
	
	// Release the lock
	//if (lock_do_i_hold(lock))
		lock_release(lock);
	
	// Put the thread into the waiting queue of the cv
	thread_sleep(cv);

	// Reacquire the lock
	lock_acquire(lock);

	// Return to user mode
	splx(spl);
}

void
cv_signal(struct cv *cv, struct lock *lock)
{

	// Check to make sure we have an actual lock
	assert(lock != NULL);
	
	// Check to make sure we have an actual cv
	assert(cv != NULL);
	
	int spl = splhigh();
	
	// Release the lock
	lock_release(lock);

	// Wake up a single thread waiting on the lock
	thread_wakeup_single(cv);
	
	// Reacquire the lock
	lock_acquire(lock);

	// Return to user mode
	splx(spl);
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	int spl;

	// Check to make sure we have an actual lock
	assert(lock != NULL);

	// Check to make sure we have an actual cv
	assert(cv != NULL);

	spl = splhigh();
	
	// Release the lock
	lock_release(lock);

	// Wake up all threads waiting on the lock
	thread_wakeup(cv);
	
	// Reacquire the lock
	lock_acquire(lock);

	// Return to user mode
	splx(spl);
}
