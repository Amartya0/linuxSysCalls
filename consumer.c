// a program that reads from a circular queue stored in share memory
// synchronized by two semaphores

// including the necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>

#define SIZE 10
#define MAX_SLEEP_TIME 5
#define TERMINATION_CODE -9999
#define SEM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_semaphore_key.txt"
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

    printf("Enter %d to clear the resources and exit the program or anything else to continue.\n", TERMINATION_CODE);
    scanf("%d", &data);
    if (data == TERMINATION_CODE)
    {
        system("pkill -f producer");
        clearResources(sem_set_id, shm_id);
        /*kill all other producer processes also. Again this will only work
        when the producer process is running with name "producer"*/
        system("pkill -f consumer");

        exit(0);
    }
    srand(time(0));
    while (1)

    {
        printf("Ready to consume...\n");
        P(sem_set_id); // decrement the value of the occupied slots
        data = q->data[q->front];
        q->front = (q->front + 1) % SIZE;
        V(sem_set_id); // increment the value of the empty slots
        printf("Consumer: %d\n", data);
        sleep(rand() % MAX_SLEEP_TIME);
    }

    // This part of the code will never be reached
    // I still keep it in case of any unexpected event
    printf("Unexpected event happened in the consumer process.\n");
    exit(1);
}