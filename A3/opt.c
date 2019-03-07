#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#define MAX 256


extern int memsize;

extern int debug;

extern struct frame *coremap;

// Path to input trace file.
extern char * tracefile;

// We use linked list implementation.
struct lnode{
    unsigned long data;
    struct lnode* next_node;
};

// Global varibales for the convience.
struct lnode* head_node = NULL;
struct lnode* tail_node = NULL;


/* Helper function to determine the number of nodes in between each use of the node to its next use,
 * i.e, the distance between the use of node "A" (A -> B -> C -> A -> A) is 2 if we start from the head.
 */
int nodes_between(struct lnode* cur_node, unsigned long vaddr) {
    unsigned long cur_addr = cur_node->data;
    int count = 0;
    
    // Keep looking at the linked list to find the next position of the current node.
    while (cur_node != NULL) {
        
        if (cur_addr != vaddr) {
            cur_node = cur_node->next_node;
            count++;
            
        } else {
            return count;
        }
    }
    
    return count;
}


/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
    int distance_of_nodes;
    int evict_idx = 0;
    int temp_max = -1;
    
    // Evict the node with the least "nodes_between", i.e, furthest used in the future.
    for (int i = 0; i < memsize; i++) {
        
        distance_of_nodes = nodes_between(head_node, (unsigned long)coremap[i].pte);
        
        // Update the furthest period.
        if (distance_of_nodes > temp_max) {
            temp_max = distance_of_nodes;
            evict_idx = i;
        }
    }
    
    return evict_idx;
}


/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
// Read the next virtual address in the linked list of the trace file.
void opt_ref(pgtbl_entry_t *p) {
    struct lnode* previous = head_node;
    head_node = head_node->next_node;
    free(previous);
}


/* Initializes any data structures needed for this
 * replacement algorithm.
 */
// Read the formatted input file to construct the info as a linked list.
void opt_init() {
    FILE* fp;
    fp = fopen(tracefile, "r");
    char type;
    
    if (fp == NULL) {
        fprintf(stderr, "Failed to open the input tracefile\n");
        exit(1);
    }
    
    char buf[MAX];
    unsigned long vaddr;
    
    // Get each line of the tracefile
    while (fgets(buf, MAX, fp) != NULL) {
        
        if (buf[0] != '=') {
            sscanf(buf, "%c %lx", &type, &vaddr);
            
            struct lnode* cur_node = malloc(sizeof(struct lnode));
            if (cur_node == NULL) {
                fprintf(stderr, "Failed to macro for current node\n");
                exit(1);
            }
            cur_node->data = vaddr;
            cur_node->next_node = NULL;
        
            if (head_node == NULL) { // First info coming from formatted string.
                // Assign the head_node and tail_node now be the first node read.
                head_node = cur_node;
                tail_node = cur_node;
            
            } else { // Keep updating the tail.
                tail_node->next_node = cur_node;
                tail_node = cur_node;
            }
        }
    }
    
    // Finish construct the linked list, close the trace file.
    int p = fclose(fp);
    if (p != 0) {
        fprintf(stderr, "Failed to close the input file\n");
        exit(1);
    }
}
