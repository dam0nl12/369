#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

static int arm;


/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int clock_evict() {
	
    while(arm < memsize) {
        
        // The ref bit is 0, replace.
        if (!(coremap[arm].pte->frame & PG_REF)) {
            return arm;
        
            // Used recently, don't replace, but reset the ref bit.
        } else {
            coremap[arm].pte->frame = coremap[arm].pte->frame & ~PG_REF;
        }
        
        arm = (arm + 1) % memsize;
        // If arm == memsize, need to sweep through the pages, again.
    }
    
    // Slience a warning.
    return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
    
    //return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
    arm = 0;
}
