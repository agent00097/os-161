/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>




struct lock *NW;
struct lock *NE;
struct lock *SW;
struct lock *SE;
struct lock *temp;
struct lock *anothertemp;
struct lock *tempone;

int NWregion = 0;
int NEregion = 0;
int SWregion = 0;
int SEregion = 0;
int carlimit = 0;
int cars_left = 0;
/*
 *
 * Constants
 *
 */

/*
 * Number of cars created.
 */

#define NCARS 20
/*static const char *all_directions[] = {"N", "E", "S", "W"};

static const char *all[] = {
        "approaching:",
        "region1:",
        "region2:",
        "region3:",
        "leaving:"
};*/

/*
 *
 * Function Definitions
 *
 */


/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

/*static void printing_car(int carnumber, int message_counter, int cardirection, int destdirection) {
        kprintf("Car#%lu is %s from %s and is going to %s\n", carnumber, all[message_counter], all_directions[cardirection], all_directions[destdirection]);
}*/

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) cardirection;
        (void) carnumber;

        lock_acquire(anothertemp);
	if (cardirection == 0) {
                //Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from North and going South\n", carnumber); 

                //Now acquring the lock for the northwest region
                
                lock_acquire(NW);
		
		NWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NW, coming from North and going South\n", carnumber);
		NWregion = 0;

                //Now acquiring lock for the Southwest region
                lock_acquire(SW);
		lock_release(NW);
                
                
		SWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SW, coming from North and going South\n", carnumber);
		SWregion = 0;
		lock_release(SW);
                
                //Car has reached it's destination
                kprintf("Car#%lu has reached South, coming from North and going South\n", carnumber);
                lock_release(anothertemp);
	}
        else if (cardirection == 1) {
                //Car direction 1, If car is taking this path, it will go to NE and then NW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from East and going West\n", carnumber); 

                //Now acquring the lock for the northwest region
                
                lock_acquire(NE);
		
		NEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NE, coming from East and going West\n", carnumber);
		NEregion = 0;

                lock_acquire(NW);
		lock_release(NE);

                //Now acquiring lock for the Southwest region
                
		NWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NW, coming from East and going West\n", carnumber);
		NWregion = 0;
		lock_release(NW);
                
                kprintf("Car#%lu has reached West, coming from East and going West\n", carnumber);
                lock_release(anothertemp);
	}
        else if (cardirection == 2) {
                //Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from South and going North\n", carnumber); 

                //Now acquring the lock for the northwest region
                
                lock_acquire(SE);
		
		SEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SE, coming from South and going North\n", carnumber);
		SEregion = 0;

                lock_acquire(NE);
		lock_release(SE);

                //Now acquiring lock for the Southwest region
                
		NEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NE, coming from South and going North\n", carnumber);
		NEregion = 0;
		lock_release(NE);
                
                kprintf("Car#%lu has reached North, coming from South and going North\n", carnumber);
                lock_release(anothertemp);
	}
        else {
                //Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from West and going East\n", carnumber); 

                //Now acquring the lock for the northwest region

                lock_acquire(SE);
                lock_acquire(SW);
		
		SWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SW, coming from West and going East\n", carnumber);
		SWregion = 0;
                
                
		lock_release(SW);

                //Now acquiring lock for the Southwest region
                
		SEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SE, coming from West and going East\n", carnumber);
		SEregion = 0;
		lock_release(SE);
                
                kprintf("Car#%lu has reached East, coming from West and going East\n", carnumber);
                lock_release(anothertemp);
	}

        lock_acquire(temp);
        cars_left--;
        lock_release(temp);

        kprintf("/////////////////////////////CARS LEFT: %d/////////////////////////////\n", cars_left);

}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
        lock_acquire(anothertemp);
        (void) cardirection;
        (void) carnumber;

        if (cardirection == 0) {
                //Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from North and going East\n", carnumber); 

                //Now acquring the lock for the northwest region
                
                //gostraight()
                lock_acquire(NW);
		
		NWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NW, coming from North and going East\n", carnumber);
		NWregion = 0;
                
                lock_acquire(SW);
		lock_release(NW);

                //Now acquiring lock for the Southwest region
                
		SWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SW, coming from North and going East\n", carnumber);
		SWregion = 0;

                lock_acquire(SE);
		lock_release(SW);

                SEregion = 1;
                kprintf("Car#%lu is in SE, coming from North and going East\n", carnumber);
                SEregion = 0;

                lock_release(SE);

                kprintf("Car#%lu has reached East, coming from North and going East\n", carnumber);
                lock_release(anothertemp);
	}
        else if (cardirection == 1) {//Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from East and going South\n", carnumber); 

                //Now acquring the lock for the northwest region
                
                
                lock_acquire(NE);
		
		NEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NE, coming from East and going South\n", carnumber);
		NEregion = 0;
                
                lock_acquire(NW);
		lock_release(NE);

                //Now acquiring lock for the Southwest region
                
		NWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NW, coming from East and going South\n", carnumber);
		NWregion = 0;

                lock_acquire(SW);
		lock_release(NW);

                SWregion = 1;
                kprintf("Car#%lu is in SW, coming from East and going South\n", carnumber);
                SWregion = 0;

                lock_release(SW);

                kprintf("Car#%lu has reached South, coming from East and going South\n", carnumber);
                lock_release(anothertemp);
	}
        else if (cardirection == 2) {
                //Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from South and going West\n", carnumber); 

                //Now acquring the lock for the northwest region
                
                
                lock_acquire(SE);
		
		SEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SE, coming from South and going West\n", carnumber);
		SEregion = 0;
                
                lock_acquire(NE);
		lock_release(SE);

                //Now acquiring lock for the Southwest region
                
		NEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in NE, coming from SOuth and going West\n", carnumber);
		NEregion = 0;

                lock_acquire(NW);
		lock_release(NE);

                NWregion = 1;
                kprintf("Car#%lu is in NW, coming from South and going West\n", carnumber);
                NWregion = 0;

                lock_release(NW);

                kprintf("Car#%lu has reached West, coming from South and going West\n", carnumber);
                lock_release(anothertemp);
	}
        else {
                //Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                kprintf("Car#%lu is approaching an intersection, coming from West and going North\n", carnumber); 

                //Now acquring the lock for the northwest region

                lock_acquire(NE);
                lock_acquire(SE);
                lock_acquire(SW);
		
		SWregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SW, coming from West and going North\n", carnumber);
		SWregion = 0;
                
		lock_release(SW);

                //Now acquiring lock for the Southwest region
                
		SEregion = 1;
                //printing out message since the car is in the new region
                kprintf("Car#%lu is in SE, coming from West and going North\n", carnumber);
		SEregion = 0;

                
		lock_release(SE);

                NEregion = 1;
                kprintf("Car#%lu is in NE, coming from West and going North\n", carnumber);
                NEregion = 0;

                lock_release(NE);

                kprintf("Car#%lu has reached North, coming from West and going North\n", carnumber);

                
                lock_release(anothertemp);
	}

        lock_acquire(temp);
        cars_left--;
        lock_release(temp);
        kprintf("/////////////////////////////CARS LEFT: %d/////////////////////////////\n", cars_left);


}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;

        if(cardirection == 0) {
                //Car direction 0, If car is taking this path, it will go to NW and then SW

                //printing out message
                
                kprintf("Car#%lu is approaching an intersection, coming from North and going West\n", carnumber);
                //Now acquring the lock for the northwest region
                lock_acquire(NW);
                NWregion = 1;
                kprintf("Car#%lu is in NW, coming from North and going West\n", carnumber);
                NWregion = 0;
                lock_release(NW);
                kprintf("Car#%lu has reached West, coming from North and going West\n", carnumber);

        } else if(cardirection == 1) {
                kprintf("Car#%lu is approaching an intersection, coming from East and going North\n", carnumber);
                //Now acquring the lock for the northwest region
                lock_acquire(NE);
                NEregion = 1;
                kprintf("Car#%lu is in NE, coming from East and going North\n", carnumber);
                NEregion = 0;
                lock_release(NE);
                kprintf("Car#%lu has reached North, coming from East and going North\n", carnumber);
                
        } else if(cardirection == 2) {

                kprintf("Car#%lu is approaching an intersection, coming from South and going East\n", carnumber);
                //Now acquring the lock for the northwest region
                lock_acquire(SE);
                SEregion = 1;
                kprintf("Car#%lu is in SE, coming from South and going East\n", carnumber);
                SEregion = 0;
                lock_release(SE);
                kprintf("Car#%lu has reached East, coming from South and going East\n", carnumber);
                
        } else{

                kprintf("Car#%lu is approaching an intersection, coming from West and going South\n", carnumber);
                //Now acquring the lock for the northwest region
                lock_acquire(SW);
                SWregion = 1;
                kprintf("Car#%lu is in SW, coming from West and going South\n", carnumber);
                SWregion = 0;
                lock_release(SW);
                kprintf("Car#%lu has reached South, coming from West and going South\n", carnumber);
        }

        lock_acquire(temp);
        cars_left--;
        lock_release(temp);
        kprintf("/////////////////////////////CARS LEFT: %d/////////////////////////////\n", cars_left);
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;
        int cardestination;
		

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
        (void) gostraight;
        (void) turnleft;
        (void) turnright;
		
		
		//lock_acquire(temp);
		
        /*
         * cardirection is set randomly. (Where the car is coming from)
         *
         * 0) North
         * 1) East
         * 2) South
         * 3) West
         */
    /*while(carlimit>2){
			;
		}
        
		carlimit++;*/

        cardirection = random() % 4;
	cardestination = random() % 3;

        if (cardestination==0){
            turnright(cardirection,carnumber);}
        else if (cardestination==2) {
            turnleft(cardirection,carnumber);}
        else if (cardestination==1) {
            gostraight(cardirection,carnumber);}
         
}
        

		/*lock_acquire(temp);
		carlimit = carlimit - 1;	
		lock_release(temp);*/        


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;

        NW = lock_create("NW");
        NE = lock_create("NE");
        SW = lock_create("SW");
        SE = lock_create("SE");
        temp = lock_create("temp");
        anothertemp = lock_create("anothertemp");
        tempone = lock_create("tempone");

        cars_left = NCARS;

        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }

        }

        while(cars_left != 0) {
                ;
        }
        return 0;
}

