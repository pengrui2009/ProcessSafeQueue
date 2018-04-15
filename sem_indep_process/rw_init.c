#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

#define NO_ELEMENTS 600
size_t shm_size = sizeof(char) * NO_ELEMENTS;

int main(int argc, char *argv[]) {

    /**
        OPENING SEMAPHORES
    */
    sem_t *reader = sem_open("readers", O_CREAT, 0600, 1);
    sem_t *writer = sem_open("writer", O_CREAT, 0600, 1);
    sem_t *mut = sem_open("mut", O_CREAT, 0600, 0);
    sem_t *start_index = sem_open("start_index", O_CREAT, 0600, 0);
    sem_t *end_index = sem_open("end_index", O_CREAT, 0600, 0);

    /**
       PRINT INFO
   */
    int count_mut;
    sem_getvalue(mut, &count_mut);
    printf("mut: %d\n", count_mut);

    int count_reader;
    sem_getvalue(reader, &count_reader);
    printf("reader: %d\n", count_reader);

    int count_writer;
    sem_getvalue(writer, &count_writer);
    printf("writer: %d\n", count_writer);

    if (count_mut == 0 && count_reader == 0 && count_writer == 0) {
        printf("NEED TO BE RESET \n");
    } else if (count_mut == 1 && count_reader == 0 && count_writer == 1) {
        printf("Already running but idle\n");
    } else if (count_mut == 1 && count_reader > 0 && count_writer == 1) {
        printf("Already running and READING\n");
    } else if (count_mut == 1 && count_reader > 0 && count_writer == 1) {
        printf("Already running and lets see\n");
    }

    if (argc > 1) {
        return 0;
    }

    /**
        OPENING SHARED MEMORY
    */
    int shm_id;
    key_t key = 123456;
    char *shm;

    // Creating a segment
    if ((shm_id = shmget(key, shm_size, IPC_CREAT | 0600)) < 0) {
        perror("shm_get error");
        exit(1);
    }

    // shm is the link the shared mem
    if ((shm = shmat(shm_id, NULL, 0)) == (char *) -1) {
        perror("error in shmat");
        exit(1);
    }

    /**
     * Initial data
     */
    sprintf(shm, " abcdefghijklmno");


    /**
        RESTING SEMPAHORES
    */
    while (true) {
        int count;
        sem_getvalue(mut, &count);
        if (count == 0) {
            break;
        }
        sem_wait(mut);
    }

    while (true) {
        int count;
        sem_getvalue(writer, &count);
        if (count == 0) {
            break;
        }
        sem_wait(writer);
    }

    while (true) {
        int count;
        sem_getvalue(reader, &count);
        if (count == 0) {
            break;
        }
        sem_wait(reader);
    }

    sem_post(writer);
    sem_post(mut);

    printf("configured\n");
    return 1;
}