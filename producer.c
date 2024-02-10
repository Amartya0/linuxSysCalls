// a program that writes into a circuler queue stored in share memory
// synchronized by two semaphores

// including the necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
// #include <signal.h>

// defining the size of the queue
#define SIZE 10
#define SEM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/sharedSemaphoreName.txt"
#define SHM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_shared_memory_key.txt"
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

// typedef void (*sighandler_t)(int);

// defining the structure of the queue
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

// global variables
// int sem_set_id, shm_id;

//  a function to remove the shared memory and the semaphores
//  void releaseResource(int signum)
//  {
//      int status;
//      // remove the shared memory
//      printf("\n");
//      status = shmctl(shm_id, IPC_RMID, NULL);
//      if (status == 0)
//      {
//          printf("Shared memory with id %d successfully released\n", shm_id);
//      }
//      else if (status == -1)
//      {
//          printf("Error releasing shared memory with id %d\n", shm_id);
//      }
//      else
//      {
//          printf("Unknown error releasing shared memory with id %d\n", shm_id);
//      }

//     // remove the semaphores
//     status = semctl(sem_set_id, 0, IPC_RMID);
//     if (status == 0)
//     {
//         printf("Semaphore set with id %d successfully released\n", sem_set_id);
//     }
//     else if (status == -1)
//     {
//         printf("Error releasing semaphore set with id %d\n", sem_set_id);
//     }
//     else
//     {
//         printf("Unknown error releasing semaphore set with id %d\n", sem_set_id);
//     }
//     exit(0);
// }

void main()
{
    // local variables
    int i, data, sem_set_id, shm_id;
    struct sembuf pop, vop;
    key_t sem_key, shm_key;
    queue *q;
    sem_arg set_sem_arg_blank, set_sem_arg_occupied;
    // sighandler_t shandler;

    // setting the signal handler
    // shandler = signal(SIGINT, releaseResource);
    // if (shandler == SIG_ERR)
    // {
    //     perror("Signal handler error");
    //     exit(1);
    // }

    // initializing the semaphores
    set_sem_arg_blank.val = SIZE - 1;
    set_sem_arg_occupied.val = 0;

    pop.sem_num = 0;
    pop.sem_op = -1;
    pop.sem_flg = SEM_UNDO;

    vop.sem_num = 1;
    vop.sem_op = 1;
    vop.sem_flg = SEM_UNDO;

    // creating the semaphore
    sem_key = ftok(SEM_KEY_FILE_PATH, 1);
    if (sem_key == -1)
    {
        perror("ftok");
        exit(1);
    }

    // creating the semaphore set containing two semaphores
    sem_set_id = semget(sem_key, 2, IPC_CREAT | 0666);
    if (sem_set_id == -1)
    {
        perror("semget");
        exit(1);
    }

    // setting the value of the first semaphore to SIZE
    if (semctl(sem_set_id, 0, SETVAL, set_sem_arg_blank) == -1)
    {
        perror("semctl");
        exit(1);
    }
    // setting the value of the second semaphore to 0
    if (semctl(sem_set_id, 1, SETVAL, set_sem_arg_occupied) == -1)
    {
        perror("semctl");
        exit(1);
    }

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

    // initializing the queue
    q->front = 0;
    q->rear = 0;

    // starting the producer
    while (1)
    {
        // redaing the data from the user

        // try to decrease the value of the first semaphore
        P(sem_set_id);
        // critical section
        printf("Ready! Enter the data to be produced: ");
        scanf("%d", &data);
        // writing the data into the queue
        q->data[q->rear] = data;
        q->rear = (q->rear + 1) % SIZE;
        // end of critical section
        // try to increase the value of the second semaphore
        V(sem_set_id);
    }
}
