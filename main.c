#include "solution.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG_MODE 1  // Set to 1 to enable debug prints, 0 to disable
#define DATA_VIEWING 1 // Set to 1 to enable testcase data-viewing, 0 to disable

#if DEBUG_MODE
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#if DATA_VIEWING
#define DV_SLEEP(...) sleep(__VA_ARGS__)
#else
#define DV_SLEEP(...)
#endif
/*
testcase.txt file format:
numthread
segments (20 int seperated by space)
seg_used
*/
zs_lock lab_lock;
/*
testcase.txt file format:
numthread
segments (20 int seperated by space)
seg_used
*/

void *scientist_routine(void *arg) {
    int id = *(int *)arg;
    DEBUG_PRINT("Scientist %d attempting to enter the lab.\n", id);
    if (scientist_enter(&lab_lock) == 0) {
        DEBUG_PRINT("Scientist %d entered the lab.\n", id);
        sleep(1); // Simulate work inside the lab
        if (scientist_exit(&lab_lock) == 0) {
            DEBUG_PRINT("Scientist %d exited the lab.\n", id);
        } else {
            DEBUG_PRINT("Error: Scientist %d could not exit the lab.\n", id);
        }
    } else {
        DEBUG_PRINT("Error: Scientist %d could not enter the lab.\n", id);
    }
    return NULL;
}

void *zombie_routine(void *arg) {
    int id = *(int *)arg;
    DEBUG_PRINT("Zombie %d attempting to enter the lab.\n", id);
    if (zombie_enter(&lab_lock) == 0) {
        DEBUG_PRINT("Zombie %d entered the lab.\n", id);
        sleep(1); // Simulate time spent inside the lab
        if (zombie_exit(&lab_lock) == 0) {
            DEBUG_PRINT("Zombie %d exited the lab.\n", id);
        } else {
            DEBUG_PRINT("Error: Zombie %d could not exit the lab.\n", id);
        }
    } else {
        DEBUG_PRINT("Error: Zombie %d could not enter the lab.\n", id);
    }
    return NULL;
}

int main() {
    int i;
    int k;
    int numthread; // number of threads
    // segments of scientists and zombies. For example, { 0, 4, 6, 8....} means 0,1,2,3 people are scientists, 4,5 people are zombies, 6,7 people are scientists...
    int segments[20];
    // how many segments are used. For example, seg_used = 3 and segments = { 0, 4, 6 ,8 ,10 ...} means onyl 0,4,6,8 are used (treating segments as length 3+1)
    int seg_used;

    FILE *fptr;
    fptr = fopen("./testcase.txt","r");
    if(fptr == NULL){
        DEBUG_PRINT("Error: cannot open test file\n");
        return 0;
    }

    if(fscanf(fptr,"%*s %*s %*s %*s %*s %*s %d", &numthread)!=1){
        DEBUG_PRINT("Error: incorret testcase format\n");
        return 0;
    };

    for(i = 0 ; i < 20 ; i++){
        if(fscanf(fptr,"%d", &segments[i])!=1){
            DEBUG_PRINT("Error: incorret testcase format\n");
            return 0;
        };
    }

    if(fscanf(fptr,"%d", &seg_used)!=1){
        DEBUG_PRINT("Error: incorret testcase format\n");
        return 0;
    };

    DEBUG_PRINT("Value of numthread is: %d \n", numthread);
    DV_SLEEP(1);
    for(i = 0 ; i < 20 ; i++){
        DEBUG_PRINT("Value of segments[%d] is: %d \n", i, segments[i]);
        DV_SLEEP(1);
    }
    DEBUG_PRINT("Value of seg_used is: %d \n", seg_used);
    DV_SLEEP(1);
    fclose(fptr);

    pthread_t threads[numthread];
    int ids[numthread];
    for(i = 0 ; i < numthread ; i++){
        ids[i] = i;
    }

    if(segments[seg_used] != numthread){
        DEBUG_PRINT("Error: testcase and segment used unmatch\n");
        return 0;
    }

    // Initialize the lab lock
    spinlock_t* lock = malloc(sizeof(spinlock_t));
    spinlock_init_ece391(lock);
    lab_lock.zombie_count = 0;
    lab_lock.scientist_count = 0;
    lab_lock.scientist_in_queue = 0;
    lab_lock.spinlock = lock;
    // Create threads for scientists and zombies
    for(k = 0 ; k < seg_used ; ++k){
        for(i = segments[k] ; i < segments[k+1]; ++i){
            if( k%2 == 0 ){
                pthread_create(&threads[i], NULL, scientist_routine, &ids[i]);
            } else {
                pthread_create(&threads[i], NULL, zombie_routine, &ids[i]);
            }
        }
    }

    // Wait for all threads to complete
    for (i = 0; i < numthread; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup lab lock
    spinlock_destroy_ece391(lock);
    free(lock);

    return 0;
}
