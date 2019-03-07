#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "traffic.h"

extern struct intersection isection;

/**
 * Populate the car lists by parsing a file where each line has
 * the following structure:
 *
 * <id> <in_direction> <out_direction>
 *
 * Each car is added to the list that corresponds with 
 * its in_direction
 * 
 * Note: this also updates 'inc' on each of the lanes
 */
void parse_schedule(char *file_name) {
    int id;
    struct car *cur_car;
    struct lane *cur_lane;
    enum direction in_dir, out_dir;
    FILE *f = fopen(file_name, "r");

    /* parse file */
    while (fscanf(f, "%d %d %d", &id, (int*)&in_dir, (int*)&out_dir) == 3) {

        /* construct car */
        cur_car = malloc(sizeof(struct car));
        cur_car->id = id;
        cur_car->in_dir = in_dir;
        cur_car->out_dir = out_dir;

        /* append new car to head of corresponding list */
        cur_lane = &isection.lanes[in_dir];
        cur_car->next = cur_lane->in_cars;
        cur_lane->in_cars = cur_car;
        cur_lane->inc++;
    }

    fclose(f);
}


/**
 * TODO: Fill in this function
 *
 * Do all of the work required to prepare the intersection
 * before any cars start coming
 * 
 */
void init_intersection() {
    struct lane *l;
    int i;
    
    for (i = 0; i < MAX_DIRECTION; i++) {
        
        // Initlialize a lock for each quadrant.
        pthread_mutex_init(&isection.quad[i], NULL);
        
        // Initialize a lane.
        l = &(isection.lanes[i]);
        pthread_mutex_init(&l->lock, NULL);
        pthread_cond_init(&l->consumer_cv, NULL);
        pthread_cond_init(&l->producer_cv, NULL);
        
        l->in_cars = NULL;
        l->out_cars = NULL;
        l->inc = 0;
        l->passed = 0;
        
        // Initialize the lane buffer with an error check.
        l->buffer = (struct car**) malloc(LANE_LENGTH * sizeof(struct car*));
        if (l->buffer == NULL){
            fprintf(stderr, "Fatal: failed to allocate space for lane_buffer.\n");
            exit(-1);
        }
        
        l->head = 0;
        l->tail = 0;
        l->capacity = LANE_LENGTH;
        l->in_buf = 0;
    }
}


/**
 * TODO: Fill in this function
 *
 * Populates the corresponding lane with cars as room becomes
 * available. Ensure to notify the cross thread as new cars are
 * added to the lane.
 * 
 */
void *car_arrive(void *arg) {
    struct lane *l = arg;
    struct car *next_car;
    
    // Keep running while there are still cars waiting.
    while (l->in_cars != NULL) {
        
        // Remove the car from in_cars.
        next_car = l->in_cars;
        l->in_cars = next_car->next;
        pthread_mutex_lock(&l->lock);
        
        // Keep waiting, while the buffer is full.
        while (l->in_buf == l->capacity){
            pthread_cond_wait(&l->producer_cv, &l->lock);
        }
        
        // Update the tail car and the tail index in buffer.
        *(l->buffer + l->tail) = next_car;
        l->tail = (l->tail + 1) % (l->capacity);
        l->in_buf++;
        
        // Notify the cross thread as the new car is added to the lane.
        pthread_cond_signal(&l->consumer_cv);
        pthread_mutex_unlock(&l->lock);
    }
    
    return NULL;
}


/**
 * TODO: Fill in this function
 *
 * Moves cars from a single lane across the intersection. Cars
 * crossing the intersection must abide the rules of the road
 * and cross along the correct path. Ensure to notify the
 * arrival thread as room becomes available in the lane.
 *
 * Note: After crossing the intersection the car should be added
 * to the out_cars list of the lane that corresponds to the car's
 * out_dir. Do not free the cars!
 *
 * 
 * Note: For testing purposes, each car which gets to cross the 
 * intersection should print the following three numbers on a 
 * new line, separated by spaces:
 *  - the car's 'in' direction, 'out' direction, and id.
 * 
 * You may add other print statements, but in the end, please 
 * make sure to clear any prints other than the one specified above, 
 * before submitting your final code. 
 */
void *car_cross(void *arg) {
    struct lane *l = arg;
    struct car *cross_car;
    struct lane *dest_lane;
    int *path;
    int i;
    
    // While there are still cars left.
    int remain_num = l->inc;
    while (remain_num != 0) {
        
        pthread_mutex_lock(&l->lock);
        
        // Keep waiting, while the buffer is empty.
        while (l->in_buf == 0) {
            pthread_cond_wait(&l->consumer_cv, &l->lock);
        }
        
        // Update the head car and the head index in buffer.
        cross_car = *(l->buffer + l->head);
        l->head = (l->head + 1) % (l->capacity);
        l->in_buf--;
        
        // We tried to move these 2 lines to the bottom, but there will be a
        // randomly error: some cars were not added into out_cars.
        pthread_cond_signal(&l->producer_cv);
        pthread_mutex_unlock(&l->lock);
        
        // Get the car's destination and path.
        dest_lane = &isection.lanes[cross_car->out_dir];
        path = compute_path(cross_car->in_dir, cross_car->out_dir);
        
        // Acquire the required locks on the path.
        for (i = 0; i < MAX_DIRECTION; i++) {
            
            if (path[i] != -1) {
                pthread_mutex_lock(&isection.quad[path[i]]);
            }
        }
        
        // Driving, once the car gets all required locks.
        printf("%d %d %d\n", cross_car->in_dir, cross_car->out_dir, cross_car->id);
        
        // Add the car into out_cars.
        cross_car->next = dest_lane->out_cars;
        dest_lane->out_cars = cross_car;
        dest_lane->passed++;
        
        // Release the required locks.
        for (i = 0; i < MAX_DIRECTION; i++) {
            
            if (path[i] != -1) {
                pthread_mutex_unlock(&isection.quad[path[i]]);
            }
        }
        
        remain_num--;
        free(path);
    }
    
    free(l->buffer);
    // Do not free the out_cars.
    
    return NULL;
}


/**
 * TODO: Fill in this function
 *
 * Given a car's in_dir and out_dir return a sorted 
 * list of the quadrants the car will pass through.
 * 
 */
int *compute_path(enum direction in_dir, enum direction out_dir) {
    int i;
    // Value at path_decider[i] will indicate whether quadrant i is required.
    int path_decider[MAX_DIRECTION];
    // The path to be returned.
    int *path = (int *) malloc(sizeof(int) * MAX_DIRECTION);
    if (path == NULL){
        fprintf(stderr, "Fatal: failed to allocate space for path.\n");
        exit(-1);
    }
    
    // C compiler (based on the verison) might have a problem, when mod a
    // negative number. By adding a 4 on the out_dir, the mod result will
    // be the same and avoid the error.
    if (out_dir - in_dir < 0) {
        out_dir += MAX_DIRECTION;
    }
    
    // dir is difference by out_dir minus in_dir, which shows the direction the
    // car is heading to.
    // 1 = right; 2 = straight; 3 = left; 4 = U-turn.
    int dir = (out_dir - in_dir) % MAX_DIRECTION;
    // Based on the piazza post, there will not be any U-turn in the test; but
    // for the convenience, we will still have it in our code.
    if (dir == 0) {
        dir = MAX_DIRECTION;
    }
    
    for (i = 0; i < dir; i++) {
        path_decider[(in_dir + i) % MAX_DIRECTION] = 1;
    }
    // Since there will not be any U-turn, this if-condition-check should always be true.
    if (dir < MAX_DIRECTION) {
        for (i = dir; i < MAX_DIRECTION; i++) {
            path_decider[(in_dir + i) % MAX_DIRECTION] = 0;
        }
    }
    
    // Sort the path by the numerical priority.
    for (i = 0; i< MAX_DIRECTION; i++) {
        
        // path_decider[i] != 0; a required lock.
        if (path_decider[i]) {
            path[i] = i;
            
            // An unrequired lock.
        } else {
            path[i] = -1;
        }
    }
    
    return path;
}
