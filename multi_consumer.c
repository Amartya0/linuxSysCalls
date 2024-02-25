/*  This is a program to:
        Consume random numbers (maximum is the capacity of the queue, here 10) from the shared queue and print them out.
        As there can be multiple instances of this program, two semaphores is used to synchronize them.
        Depending on the state (explained below) of semaphore the program either gets its' desired numbers in the queue or wait for the semaphore to change and try the same again.
        After successfully consuming some data it will go to sleep for a random amount of time (a maximum is given for convenience).

    However the creation of the shared memory segment, the creation and initialization of the semaphores is necessary prior to the execution of any instances of this program.
    Use thr manager program to do as such.
*/

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>

// Defining the constants (macros)
#define SIZE 10
#define MAX_SLEEP_TIME 5
#define SEM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_semaphore_key.txt"
#define SHM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_shared_memory_key.txt"

// Defining the operation on semaphores
#define P(s) semop(s, &pop, 1) // Macro for semaphore P operation
#define V(s) semop(s, &vop, 1) // Macro for semaphore V operation

// Defining the structure of the queue
typedef struct
{
    int front;      // Index of the front of the queue
    int rear;       // Index of the rear of the queue
    int data[SIZE]; // Array to store data in the queue
} queue;

// Defining the union for semaphores
typedef union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} sem_arg;

// Function prototypes

void updateSembuf(struct sembuf *pop, struct sembuf *vop, int num_items);

// Driver function
void main()
{
    // Declaring the local variables
    int sem_set_id, shm_id, i, data[SIZE], num_items;
    struct sembuf pop, vop;
    key_t sem_key, shm_key;
    queue *q;

    // Generating keys for shared memory and semaphore using ftok
    shm_key = ftok(SHM_KEY_FILE_PATH, 1);
    sem_key = ftok(SEM_KEY_FILE_PATH, 1);
    if (shm_key == -1 || sem_key == -1)
    {
        perror("1: ftok");
        exit(1);
    }

    // Getting shared memory and semaphore set IDs
    shm_id = shmget(shm_key, sizeof(queue), 0);
    sem_set_id = semget(sem_key, 2, 0);

    if (shm_id == -1 || sem_set_id == -1)
    {
        perror("2: shmget/semget");
        exit(1);
    }
    // Attaching shared memory to process address space
    q = (queue *)shmat(shm_id, 0, 0);

    while (1)
    {
        // Generating number of items to consume randomly
        num_items = rand() % SIZE + 1;
        printf("Trying to consume %d items\n", num_items);

        // Updating semaphore operations
        updateSembuf(&pop, &vop, num_items);
        P(sem_set_id); // Wait operation on semaphore

        // Consuming items from the queue
        for (i = 0; i < num_items; i++)
        {
            data[i] = q->data[q->front];
            q->front = (q->front + 1) % SIZE;
        }
        V(sem_set_id); // Signal operation on semaphore

        // Displaying consumed data
        printf("Consumed data:\t");
        for (i = 0; i < num_items; i++)
        {
            printf("\t%d", data[i]);
        }
        printf("\n");

        // Sleeping for random time before next iteration
        printf("Sleeping...\n");
        sleep(rand() % MAX_SLEEP_TIME + 1);
    }
}

/* Function to update semaphore operations
    This is where the consumer program, varies from the producer program. The operations are opposite to the producer program.
    Here we wait for the 1st semaphore (occupied slots) to be available and then consume the data. And then we signal the 2nd semaphore (empty slots) to be incremented.**/
void updateSembuf(struct sembuf *pop, struct sembuf *vop, int num_items)
{
    pop->sem_num = 1;         // semaphore number for occupied slots
    pop->sem_op = -num_items; // decrement empty slots
    pop->sem_flg = SEM_UNDO;

    vop->sem_num = 0;        // semaphore number for empty slots
    vop->sem_op = num_items; // increment occupied slots
    vop->sem_flg = SEM_UNDO;
}
