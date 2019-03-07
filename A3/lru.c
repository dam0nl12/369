#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

// Implement LRU based on the option 1 in the lecture note "L14-PageReplacement."
// A stack will be too complex to implement and expensive to run.

// Time counter.
static int time;

// Time stamp every reference.
static int *time_stamps;


/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int lru_evict() {
    int evict_i = 0;
    // At least 1 page will be in the memory when we call evict(), so this will not cause any error.
    int oldest_time_stamp = time_stamps[0];
    
    // Need to examine every page on eviction to find the oldest one.
    // We start the loop from the 2nd element, so time_stamps[0] will not be compared to itself;
    for (int i = 1; i < memsize; i++) {
        
        if (oldest_time_stamp > time_stamps[i]) {
            evict_i = i;
            oldest_time_stamp = time_stamps[i];
        }
    }
    
	return evict_i;
}


/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
    // Update to the current time.
    time++;
    time_stamps[p->frame >> PAGE_SHIFT] = time;
    
	//return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
    time = 0;
    
    // Question: memory leak?
    // Index of an element = p's frame number, hex to decimal;
    // value of an element = last reference time.
    time_stamps = calloc(memsize, sizeof(int));
}
