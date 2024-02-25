/*  This is a program to:
        Produce random numbers (maximum is the capacity of the queue, here 10) of random numbers and store them in the shared queue.
        As there can be multiple instances of this program, two semaphores is used to synchronize them.
        Depending on the state (explained below) of semaphore the program either puts its' desired numbers in the queue or wait for the semaphore to change and try the same again.
        After successfully producing some data it will go to sleep for a random amount of time (a maximum is given for convenience).

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
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

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

    // Creating the keys for the shared memory and the semaphore set
    shm_key = ftok(SHM_KEY_FILE_PATH, 1);
    sem_key = ftok(SEM_KEY_FILE_PATH, 1);
    if (shm_key == -1 || sem_key == -1)
    {
        perror("1: ftok");
        exit(1);
    }

    // Getting the existing shared memory and semaphore set, not creating new ones. This part depends on the manager program being ran prior to this.
    shm_id = shmget(shm_key, sizeof(queue), 0); // The last argument is 0, indicating getting the existing shared memory segment.
    sem_set_id = semget(sem_key, 2, 0);         // The last argument is 0, indicating getting the existing semaphore set.

    if (shm_id == -1 || sem_set_id == -1)
    {
        // If the manager program is not ran prior to this, we will get the error message "2: shmget/semget: No such file or directory." from here.
        perror("2: shmget/semget");
        exit(1);
    }
    // Attaching the shared memory segment to the process's address space
    q = (queue *)shmat(shm_id, 0, 0);
    srand(time(0));

    // Infinite loop to produce the random numbers and store them in the shared queue and sleep for a random amount of time
    while (1)
    {
        num_items = rand() % SIZE + 1;       // Random number of items to produce
        updateSembuf(&pop, &vop, num_items); // Update the sembuf structure according to the number of items to produce, see the function definition for more details.

        for (i = 0; i < num_items; i++) // Loop to produce the required number of data.
        {
            data[i] = rand() % 100 + 1;
        }

        printf("Trying to produce %d items\n", num_items);

        // Wait for the empty slots to be available and then produce the data. This is the critical section.
        P(sem_set_id);
        for (i = 0; i < num_items; i++)
        {
            q->data[q->rear] = data[i];     // Store the data in the queue
            q->rear = (q->rear + 1) % SIZE; // Update the rear pointer of the queue by circular increment of one.
            /* We do not check if the current place is empty or not or the number of item is less than size or not, as they are automatically done the semaphore operations*/
        }
        V(sem_set_id); // Signal the occupied slots to be incremented. This is the end of the critical section.

        // Print the produced data for convenance and sleep for a random amount of time
        printf("Produced data:\t");
        for (i = 0; i < num_items; i++)
        {
            printf("\t%d", data[i]);
        }
        printf("\n");
        printf("Sleeping...\n");
        sleep(rand() % MAX_SLEEP_TIME + 1);
    }
}

/* A function to update the sembuf structure according to the number of items to produce. this is the heart of the program.
    This function takes the following arguments:
       1. A pointer to the pop structure
       2. A pointer to the vop structure
       3. The number of items to produce
    In the P operation we decrement the 0th semaphore (which represents empty slots) by the number of items to produce.
    This will make this program wait (not busy waiting) for the empty slots to be available.
    In the V operation we increment the 1st semaphore (which represents occupied slots) by the number of items to produce.

   This function does not return anything.
*/
void updateSembuf(struct sembuf *pop, struct sembuf *vop, int num_items)
{
    pop->sem_num = 0;         // semaphore number for empty slots
    pop->sem_op = -num_items; // decrement empty slots
    pop->sem_flg = SEM_UNDO;

    vop->sem_num = 1;        // semaphore number for occupied slots
    vop->sem_op = num_items; // increment occupied slots
    vop->sem_flg = SEM_UNDO;
}