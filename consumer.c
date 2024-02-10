// a program that reads from a circuler queue stored in share memory
// synchronized by two semaphores

// including the necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>

// defining the size of the queue
#define SIZE 10
#define SEM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/sharedSemaphoreName.txt"
#define SHM_KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_shared_memory_key.txt"

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

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

void main()
{
    // local variables
    int sem_set_id, shm_id, i, data;
    struct sembuf pop, vop;
    key_t sem_key, shm_key;
    queue *q;
    sem_arg set_sem_arg_blank, set_sem_arg_occupied;

    // initializing the semaphores
    set_sem_arg_blank.val = SIZE - 1;
    set_sem_arg_occupied.val = 0;

    pop.sem_num = 1;
    pop.sem_op = -1;
    pop.sem_flg = SEM_UNDO;

    vop.sem_num = 0;
    vop.sem_op = 1;
    vop.sem_flg = SEM_UNDO;

    // creating the semaphore
    sem_key = ftok(SEM_KEY_FILE_PATH, 1);
    if (sem_key == -1)
    {
        perror("ftok");
        exit(1);
    }
    sem_set_id = semget(sem_key, 2, 0666 | IPC_CREAT);
    if (sem_set_id == -1)
    {
        perror("semget");
        exit(1);
    }
    // check id the semaphores are already created
    // this happens when the producer is created first
    // and asiigns the values to the semaphores if they are not created

    semctl(sem_set_id, 0, SETVAL, set_sem_arg_blank);

    semctl(sem_set_id, 1, SETVAL, set_sem_arg_occupied);

    // creating the shared memory
    shm_key = ftok(SHM_KEY_FILE_PATH, 1);
    if (shm_key == -1)
    {
        perror("ftok");
        exit(1);
    }
    shm_id = shmget(shm_key, sizeof(queue), 0666 | IPC_CREAT);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(1);
    }
    q = (queue *)shmat(shm_id, 0, 0);
    if (q == (queue *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // reading from the queue
    while (1)

    {
        P(sem_set_id);
        printf("Ready to consume: ");
        getchar();
        data = q->data[q->front];
        q->front = (q->front + 1) % SIZE;
        printf("Consumer: %d\n", data);
        V(sem_set_id);
    }
}