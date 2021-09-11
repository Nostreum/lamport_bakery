#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/uio.h>
#include <unistd.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#ifndef NB_THREADS
#define NB_THREADS 4
#endif

void *lamport(void *pid);
void take_spinlock(int id);
void free_spinlock(int id);
void fail(int i, int number_i, int j, int number_j);

struct lamport_lock {

    unsigned char number[NB_THREADS];
// Paddings can be used to force these variables to *not be* on the same cacheline
#ifdef PADDING
    unsigned char padding0[100];
#endif
    unsigned char entering[NB_THREADS];
#ifdef PADDING
    char padding1[100];
#endif
    unsigned char lock[NB_THREADS];

};

// shared lock structure
struct lamport_lock lp_lock;

int main(int argc, char *argv[]) {

    pthread_t thread[NB_THREADS];

// Usefull to see the entering/number/lock addresses
#ifdef GET_ADDRESS
    printf("entering address = %hx \n", &lp_lock.entering);
    printf("number address = %hx \n", &lp_lock.number);
    printf("lock address = %hx \n", &lp_lock.lock);
    scanf("Continue...");
#endif

    // Thread creation. Call lamport function with thread_number as a parameter.
    for (int i=0; i<NB_THREADS; i++) {
        int status = pthread_create( &thread[i], NULL, &lamport, (void*)i);
        if (status != 0) {
            printf (" Failed to create threads \n ");
            exit(1);
        }
    }

    // Wait all threads before closing
    for (int i=0; i<NB_THREADS; i++)
        pthread_join(thread[i], NULL);

    return 0;
}

// Called when a mismatch is detected.
// Simply print the 2 faulting threads
void fail(int i, int number_i,int j, int number_j) {

    printf ("THREAD %d and THREAD %d are both in critical section \n", i, j);
    printf ("   number[%d]=%d, number[%d]=%d \n", i, number_i, j, number_j);
    exit(1);

}

// Insert DMB SR or SY
void dmb() {
#ifdef DMB_ST_ENABLED 
    __asm__("dmb st");
#endif
#ifdef DMB_SY_ENABLED 
    __asm__("dmb sy");
#endif
}

void *lamport(void *pid) {

    int id = (int)pid;

// start of the loop
start:

    // wait a be to makes sure sometimes there is no-one inside the critical section
    usleep(1);

    take_spinlock(id);
    
    // =================
    // Critical Section
    // =================

    lp_lock.lock[id] = 1;

    // Check if two threads are in the critical section (lock it set)
    for (int j=0; j<NB_THREADS; j++)
        if (j!=id && lp_lock.lock[j]) {
            fail(j, lp_lock.number[j], id, lp_lock.number[id]);
        }

    lp_lock.lock[id] = 0;

    free_spinlock(id);

    //
    // =================
    // End Critical Section
    // =================

    goto start;
}

void take_spinlock(int id) {

    // Set entering bit
    lp_lock.entering[id] = 1;
    // DMB to order younger loads
    dmb();

    // Get max number value between thread
    int max = 0;
    for (int i=0; i<NB_THREADS; i++)
        max = MAX(max, lp_lock.number[i]);

    // Set own number to 1+max (the lowest get the higher priority at the end)
    lp_lock.number[id]   = 1+max;
    // DMB to order younger loads
    dmb();

    // Clear entering bit
    lp_lock.entering[id] = 0;

    // Check that no other thread are:
    //  - doing the entering phase
    //  - entering the critical section with more priority
    //  - already in the critical section
    for (int j=0; j<NB_THREADS; j++) {

        while(lp_lock.entering[j] == 1){}

        dmb();

        while(lp_lock.number[j] != 0    && (   (lp_lock.number[j] <  lp_lock.number[id])
                                           ||  (lp_lock.number[j] == lp_lock.number[id] && j < id))) {
        }
    }

    dmb();
}

void free_spinlock(int id) {
    lp_lock.number[id] = 0;
    dmb();
}
