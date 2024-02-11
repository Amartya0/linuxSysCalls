// a program that writes into a circular queue stored in share memory
// synchronized by two semaphores

// including the necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>

// defining the size of the queue
#define SIZE 10
// define a file path for the key for semaphores
#define SEM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/sharedSemaphoreName.txt"
// define a file path for the key for shared memory
#define SHM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_shared_memory_key.txt"
// define a termination code
#define TERMINATION_CODE -9999

// defining the P and V operations for the semaphores
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

// defining the structure of the queue
typedef struct
{
    int front;
    int rear;
    int data[SIZE];
} queue;

// defining the union for the semctl function i.e., the attributes for a
// particular semaphore
typedef union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} sem_arg;

// a fuction to clear resources
void clearResources(int sem_set_id, int shm_id)
{
    // removing the semaphore set
    if (semctl(sem_set_id, 0, IPC_RMID) == -1)
    {
        perror("semctl");
        exit(1);
    }
    printf("Semaphore set  with id %d removed.\n", sem_set_id);

    // removing the shared memory segment
    if (shmctl(shm_id, IPC_RMID, 0) == -1)
    {
        perror("shmctl");
        exit(1);
    }
    printf("Shared memory segment with id %d removed.\n", shm_id);
}

void main()
{
    // local variables
    // sem_set_id: the id of the semaphore set
    // shm_id: the id of the shared memory segment
    // i: a counter variable
    // data: the data to be written into the queue
    /*num_already_present_data: the number of data already present in the queue,
    0 if the queue is empty*/
    int sem_set_id, shm_id, i, data, num_already_present_data;
    // declaring the sembuf structures for the P and V operations
    struct sembuf pop, vop;
    // declaring the keys for the semaphore and the shared memory
    key_t sem_key, shm_key;
    // declaring the shared memory queue as a pointer of type queue defined above
    queue *q;
    // declaring the semun structures for the semctl function
    /*set_sem_arg_blank: the value of the first semaphore. This keep track of the number of
    empty slots in the queue.
    set_sem_arg_occupied: the value of the second semaphore. This keep track of the number of
    occupied slots in the queue*/
    sem_arg set_sem_arg_blank, set_sem_arg_occupied;

    // creating the shared memory
    shm_key = ftok(SHM_KEY_FILE_PATH, 1);
    if (shm_key == -1)
    {
        perror("ftok");
        exit(1);
    }

    // creating the shared memory segment
    shm_id = shmget(shm_key, sizeof(queue), IPC_CREAT | 0777);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(1);
    }

    // attaching the shared memory segment to the local variable
    q = (queue *)shmat(shm_id, 0, 0);
    if (q == (queue *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // Check for already existing queue, if not then initialize the queue.
    // This is for the case when there is already a queue present in the shared memory
    // This may happen due the following reasons:
    /* 1. The producer process was terminated without clearing the resources and the consumer
    process was not able to consume all the data.
    2. Multiple producer processes are running at the same time.*/

    if (q->front == q->rear)
    {
        q->front = 0;
        q->rear = 0;
        num_already_present_data = 0;
    }
    else
    {
        // get the difference between the front and rear of the queue
        num_already_present_data = (q->rear - q->front);
    }

    // initializing the semaphores
    /*  If the producer process is running for the first time, then the value of the first
    semaphore is set to SIZE-1 and the value of the second semaphore is set to 0.
        Any other cases the value of the first semaphore is set to SIZE-1 - num_already_present_data
    and the value of the second semaphore is set to num_already_present_data.*/
    set_sem_arg_blank.val = SIZE - 1 - num_already_present_data;
    set_sem_arg_occupied.val = num_already_present_data;

    /*In the producer process, we decrease the value of the first semaphore before writing into
    the queue. That is why in P operation we decrease the value of the first semaphore by one.
    We increase the value of the second semaphore after writing into the queue
    i.e., in the V operation we increase the value of second semaphore by one */

    pop.sem_num = 0; // semaphore number for empty slots
    pop.sem_op = -1; // decrement empty slots
    pop.sem_flg = SEM_UNDO;

    vop.sem_num = 1; // semaphore number for occupied slots
    vop.sem_op = 1;  // increment occupied slots
    vop.sem_flg = SEM_UNDO;

    // creating the semaphore
    sem_key = ftok(SEM_KEY_FILE_PATH, 1);
    if (sem_key == -1)
    {
        perror("ftok");
        exit(1);
    }

    // creating the semaphore set containing two semaphores
    sem_set_id = semget(sem_key, 2, IPC_CREAT | 0777);
    if (sem_set_id == -1)
    {
        perror("semget");
        exit(1);
    }

    // We are checking if the semaphores values are already been set
    // Again this may happen reasons mentioned above when we are checking for the queue
    // If the semaphore has already been set then we don't need to set it again
    // otherwise we set he values as mentioned above

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
    /* This part was not asked in the question but I have added it so that
    the user can clear the resources, Otherwise the shared memory and semaphores
    will be present in the system even after the program is terminated.
    This is a very simple mechanism, we use a particular integer value to clear the resources.
    The producer always read inputs anyways, we just keep checking if the input is TERMINATION_CODE,
    if it is then we clear the resources and exit the program.
    However, I can not find any robust way to kill all the consumer processes also, So the user is
    asked to make sure that no consumer process is running before clearing the resources.
    However, I have added a line of code to kill all the consumer processes running with the name "consumer"
    */
    printf("The producer: Type %d to clear resources and quit, but make sure no consumer process is running!\n", TERMINATION_CODE);
    // starting the producer
    while (1)
    {

        printf("Ready! Enter the data to be produced: ");
        scanf("%d", &data);
        if (data == TERMINATION_CODE)
        {
            system("pkill -f consumer");
            // This will only work when the consumer process is running with name "consumer"
            clearResources(sem_set_id, shm_id);
            /*kill all other producer processes also. Again this will only work
            when the producer process is running with name "producer"*/
            system("pkill -f producer");
            exit(0);
        }
        // try to decrease the value of the first semaphore
        P(sem_set_id);
        // critical section
        // writing the data into the queue
        q->data[q->rear] = data;
        q->rear = (q->rear + 1) % SIZE;
        // end of critical section
        // try to increase the value of the second semaphore
        V(sem_set_id);
    }

    // This part of the code will never be reached
    // I still keep it in case of any unexpected event
    printf("Unexpected event happened in the producer process.\n");
    return;
}
