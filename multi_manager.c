/* Problems / TO DO:
    1.  If one running program is terminated with Ctrl+C, the other running program will not be terminated.
    But they will start malfunctioning as semaphores are reset to the initial values.
    2.  When the queue is full, the get status function will not work properly. It will print the queue as empty.
*/

/*This is a program to:
    1. Create the shared memory segment, create and set up semaphore set for the producer and consumer programs.
    2. To see status of the shared queue, and the semaphores.
    3. To release the shared memory segment and destroy the semaphore set when the work is done.

This program is ran using the following commands:
    1. ./multi_manager 0 to initialize
    2. ./multi_manager 1 to get status
    3. ./multi_manager 2 to terminate and clear

*/

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

// Defining the constants (macros)
#define SIZE 10
#define SEM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_semaphore_key.txt"
#define SHM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_shared_memory_key.txt"

// Defining the structure of the queue
typedef struct
{
    int front;      // Index of the front of the queue
    int rear;       // Index of the rear of the queue
    int data[SIZE]; // Array to store data in the queue
} queue;

// Defining the union for the semaphores
typedef union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} sem_arg;

// Function prototypes
void initialize(queue *q, int shm_id, int sem_set_id);
void get_status(queue *q, int shm_id, int sem_set_id);
void clear_and_terminate(int shm_id, int sem_set_id);

// Driver function
void main(int argc, char *argv[])
{
    // Check if the number of arguments is correct
    if (argc != 2)
    {
        // if not, print the usage and exit
        printf("Usage: %s <1/2/3>\n", argv[0]);
        exit(1);
    }

    // Declare the local variables
    int shm_id, sem_set_id;
    key_t shm_key, sem_key;
    queue *q;

    // Create the keys for the shared memory and the semaphores
    shm_key = ftok(SHM_KEY_FILE_PATH, 1);
    sem_key = ftok(SEM_KEY_FILE_PATH, 1);
    if (shm_key == -1 || sem_key == -1)
    {
        perror("1: ftok");
        exit(1);
    }

    // Create the shared memory and the semaphore set
    shm_id = shmget(shm_key, sizeof(queue), IPC_CREAT | 0777);
    sem_set_id = semget(sem_key, 2, IPC_CREAT | 0777);
    if (shm_id == -1 || sem_set_id == -1)
    {
        perror("2: shmget/semget");
        exit(1);
    }

    // Check the value of the argument and call the appropriate function for the operation
    if (atoi(argv[1]) == 1)
    {
        // If the argument is 1, call the initialize function, see the function definition for more details.
        initialize(q, shm_id, sem_set_id);
    }
    else if (atoi(argv[1]) == 2)
    {
        // If the argument is 2, call the get_status function, see the function definition for more details.
        get_status(q, shm_id, sem_set_id);
    }
    else if (atoi(argv[1]) == 3)
    {
        // If the argument is 3, call the clear_and_terminate function, see the function definition for more details.
        clear_and_terminate(shm_id, sem_set_id);
    }
    else
    {
        // If the argument does not match any of the above, print the usage and exit
        printf("Usage: %s <1/2/3>\n", argv[0]);
        exit(1);
    }
}

// Function definitions

/* A function to initialize the shared memory and the semaphores.
   This function takes the following arguments:
       1. A pointer to the queue structure
       2. The shared memory id
       3. The semaphore set id
   This function sets the front and rear of the queue to 0 and initializes the semaphores to the required values.
   This function does not return anything. If everything goes successfully it prints a message in the terminal and exits with 0, otherwise exits with 1. */
void initialize(queue *q, int shm_id, int sem_set_id)
{
    q = (queue *)shmat(shm_id, 0, 0);
    q->front = q->rear = 0;
    shmdt(q);

    /*The 0th semaphore is for empty slots, and the 1st semaphore is for occupied slots.
    So the 0th semaphore is initialized to the size of the queue, and the 1st semaphore is initialized to 0.*/
    if (semctl(sem_set_id, 0, SETVAL, SIZE) == -1)
    {
        perror("3: semctl");
        exit(1);
    }
    if (semctl(sem_set_id, 1, SETVAL, 0) == -1)
    {
        perror("4: semctl");
        exit(1);
    }
    printf("Initialized\n");
    exit(0);
}

/* A function to see the status of the shared memory and the semaphores.
   This function takes the following arguments:
       1. A pointer to the queue structure
       2. The shared memory id
       3. The semaphore set id
   This function prints the queue, the front and rear of the queue, and the values of the semaphores.
   This function does not return anything. It exits with 0 on success. */
void get_status(queue *q, int shm_id, int sem_set_id)
{
    q = (queue *)shmat(shm_id, 0, 0);
    printf("Queue: ");
    for (int i = q->front; i != q->rear; i = (i + 1) % SIZE)
    {
        printf("%d ", q->data[i]);
    }
    printf("\n");
    printf("Front: %d\n", q->front);
    printf("Rear: %d\n", q->rear);
    shmdt(q);
    printf("Semaphores: empty: %d\toccupied: %d\n", semctl(sem_set_id, 0, GETVAL), semctl(sem_set_id, 1, GETVAL));
    exit(0);
}

/* A function to clear and terminate the shared memory and the semaphores.
   This function takes the following arguments:
       1. The shared memory id
       2. The semaphore set id
   This function terminates and clears the shared memory and the semaphore set.
   It also terminates the producer and consumer programs and finally itself, which has the name multi_producer, multi_consumer, and multi_manager.
   This is not an ideal way to terminate programs, but it is done here for simplicity.
   This function does not return anything. It exits with 0 on success or 1 otherwise. */
void clear_and_terminate(int shm_id, int sem_set_id)
{
    system("pkill -f ./multi_consumer");
    system("pkill -f ./multi_producer");
    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("5: shmctl");
        exit(1);
    }
    if (semctl(sem_set_id, 0, IPC_RMID) == -1)
    {
        perror("6: semctl");
        exit(1);
    }
    system("pkill -f ./multi_manager");
    exit(0);
}
