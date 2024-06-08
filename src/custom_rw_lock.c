#ifndef CUSTOM_RW_LOCK_H
#define CUSTOM_RW_LOCK_H

#include "custom_rw_lock.h"
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

/**
 * gets lock for reading an element
 * Call this function before reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void get_read_lock(struct lock_custom *lock_ptr) {

    int count;

    sem_wait(lock_ptr->mut);        // lock(mutex)
    sem_post(lock_ptr->reader);    //read_count++

    sem_getvalue(lock_ptr->reader, &count);
    if (count == 1) {
        sem_wait(lock_ptr->writer);
    }

    sem_post(lock_ptr->mut);
}

/**
 * releases lock for reading an element
 * Call this function after reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void realease_read_lock(struct lock_custom *lock_ptr) {

    int count;

    sem_wait(lock_ptr->mut);        // lock(mutex)
    sem_wait(lock_ptr->reader);    //read_count--

    sem_getvalue(lock_ptr->reader, &count);
    if (count == 0) {
        sem_post(lock_ptr->writer);
    }

    sem_post(lock_ptr->mut);
}

/**
 * gets lock for writing an element
 * Call this function before writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void get_write_lock(struct lock_custom *lock_ptr) {
    sem_wait(lock_ptr->writer);
}

/**
 * release lock after writing an element
 * Call this function after writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void realease_write_lock(struct lock_custom *lock_ptr) {
    sem_post(lock_ptr->writer);
}


/**
 * Prints the state of the locks
 * @param lock the lock_custom concerned
 */
void print_lock(struct lock_custom *lock_ptr) {

    int count_mut;
    sem_getvalue(lock_ptr->mut, &count_mut);
    printf("mut: %d\n", count_mut);

    int count_reader;
    sem_getvalue(lock_ptr->reader, &count_reader);
    printf("reader: %d\n", count_reader);

    int count_writer;
    sem_getvalue(lock_ptr->writer, &count_writer);
    printf("writer: %d\n", count_writer);

}


struct lock_custom *init_lock_custom(struct lock_custom *lock_ptr, char *id, int sync) {

    lock_ptr = (struct lock_custom *) malloc(sizeof(struct lock_custom));

    char readers[100] = "readers";
    lock_ptr->reader = sem_open(strcat(readers, id), O_CREAT, 0600, 1);

    char writer[100] = "writer";
    lock_ptr->writer = sem_open(strcat(writer, id), O_CREAT, 0600, 1);

    char mut[100] = "mut";
    lock_ptr->mut = sem_open(strcat(mut, id), O_CREAT, 0600, 1);

    if (sync == 0) {
        // sem_post(lock->writer);
        // sem_post(lock->mut);
        sem_wait(lock_ptr->reader);
    }

    return lock_ptr;
}


#endif /*CUSTOM_RW_LOCK_H*/