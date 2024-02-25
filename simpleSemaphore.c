#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUMBER_OF_SEMAPHORES 1
#define KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_semaphore_key.txt"
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

int main()
{

    struct sembuf pop, vop;
    key_t sem_key;
    pid_t pidFork;
    int sem_id;

    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
        struct seminfo *__buf;
    } setvalArg;

    setvalArg.val = 1;

    pop.sem_num = 0;
    pop.sem_op = -1;
    pop.sem_flg = SEM_UNDO;

    vop.sem_num = 0;
    vop.sem_op = 1;
    vop.sem_flg = SEM_UNDO;

    sem_key = ftok(KEY_FILE_PATH, 1);
    if (sem_key == -1)
    {
        perror("ftok");
        exit(1);
    }

    sem_id = semget(sem_key, NUMBER_OF_SEMAPHORES, IPC_CREAT | 0777);
    if (sem_id == -1)
    {
        perror("semget");
        exit(1);
    }

    if (semctl(sem_id, 0, SETVAL, setvalArg) == -1)
    {
        perror("semctl");
        exit(1);
    }

    pidFork = fork();
    if (pidFork == -1)
    {
        perror("fork");
        exit(1);
    }

    if (pidFork == 0)
    {
        printf("Child process is waiting for the semaphore\n");
        P(sem_id);
        printf("Child process has acquired the semaphore\n");
        sleep(5);
        printf("Child process is releasing the semaphore\n");
        V(sem_id);
    }
    else
    {
        printf("Parent process is waiting for the semaphore\n");
        P(sem_id);
        printf("Parent process has acquired the semaphore\n");
        sleep(5);
        printf("Parent process is releasing the semaphore\n");
        V(sem_id);
    }
    wait(NULL);
    // removing the semaphore
    if (semctl(sem_id, 0, IPC_RMID) == -1)
    {
        perror("semctl");
        exit(1);
    }
    return 0;
}
