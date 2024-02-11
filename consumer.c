// a program that reads from a circular queue stored in share memory
// synchronized by two semaphores

// including the necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>

#define SIZE 10
#define SEM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/sharedSemaphoreName.txt"
#define SHM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_shared_memory_key.txt"

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

typedef struct
{
    int front;
    int rear;
    int data[SIZE];
} queue;

typedef union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} sem_arg;

void main()
{
    // local variables
    int sem_set_id, shm_id, i, data, num_already_present_data;
    struct sembuf pop, vop;
    key_t sem_key, shm_key;
    queue *q;
    sem_arg set_sem_arg_blank, set_sem_arg_occupied;

    // creating the shared memory
    shm_key = ftok(SHM_KEY_FILE_PATH, 1);
    if (shm_key == -1)
    {
        perror("ftok");
        exit(1);
    }
    // Create or get shared memory segment
    shm_id = shmget(shm_key, sizeof(queue), 0777 | IPC_CREAT);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(1);
    }
    // Attach shared memory segment to local variable
    q = (queue *)shmat(shm_id, 0, 0);
    if (q == (queue *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // get the difference between the front and rear of the queue
    num_already_present_data = (q->rear - q->front);

    set_sem_arg_blank.val = SIZE - 1 - num_already_present_data;
    set_sem_arg_occupied.val = num_already_present_data;

    /*Though in most places, consumer has same code as producer, here is the difference:
    AS consumer is reading one value from the queue, and discarding it, it will try to
    decrease the value of the first semaphore (which is the number of empty slots in the queue)
    and increase the value of the second semaphore (which is the number of occupied slots in the queue)*/

    pop.sem_num = 1; // semaphore number for occupied slots
    pop.sem_op = -1; // decrement occupied slots
    pop.sem_flg = SEM_UNDO;

    vop.sem_num = 0; // semaphore number for empty slots
    vop.sem_op = 1;  // increment empty slots
    vop.sem_flg = SEM_UNDO;

    // creating the semaphore
    sem_key = ftok(SEM_KEY_FILE_PATH, 1);
    if (sem_key == -1)
    {
        perror("ftok");
        exit(1);
    }
    sem_set_id = semget(sem_key, 2, 0777 | IPC_CREAT);
    if (sem_set_id == -1)
    {
        perror("semget");
        exit(1);
    }

    // check if the semaphore has already been initialized
    if (semctl(sem_set_id, 0, GETVAL) != -1)
    {
        // setting the value of the first semaphore to SIZE
        if (semctl(sem_set_id, 0, SETVAL, set_sem_arg_blank) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }

    if (semctl(sem_set_id, 1, GETVAL) != -1)
    {
        // setting the value of the second semaphore to 0
        if (semctl(sem_set_id, 1, SETVAL, set_sem_arg_occupied) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }

    // reading from the queue

    while (1)

    {
        P(sem_set_id); // decrement the value of the occupied slots
        printf("Ready to consume: ");
        getchar(); // just to simplify waiting for user input
        data = q->data[q->front];
        q->front = (q->front + 1) % SIZE;
        printf("Consumer: %d\n", data);
        V(sem_set_id); // increment the value of the empty slots
    }

    // This part of the code will never be reached
    // I still keep it in case of any unexpected event
    printf("Unexpected event happened in the producer process.\n");
    return;
}