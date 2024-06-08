#ifndef CUSTOM_RW_LOCK_H
#define CUSTOM_RW_LOCK_H

#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

#define EVERYTHING 1 // Used with print_queue
#define ONLY_DATA 0 // Used with print queue


/**
 * lock_custom represents set of locks required for the safety of each queue element
 */
struct lock_custom {
    sem_t *reader;
    sem_t *writer;
    sem_t *mut;
};

/**
 * gets lock for reading an element
 * Call this function before reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void get_read_lock(struct lock_custom *lock);

/**
 * releases lock for reading an element
 * Call this function after reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void realease_read_lock(struct lock_custom *lock);

/**
 * gets lock for writing an element
 * Call this function before writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void get_write_lock(struct lock_custom *lock);

/**
 * release lock after writing an element
 * Call this function after writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void realease_write_lock(struct lock_custom *lock);

/**
 * Prints the state of the locks
 * @param lock the lock_custom concerned
 */
void print_lock(struct lock_custom *lock);


struct lock_custom *init_lock_custom(struct lock_custom *lock, char *id, int sync);


#endif /*CUSTOM_RW_LOCK_H*/