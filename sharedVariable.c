#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>

typedef void (*sighandler_t)(int);

// Define the global shared memory id
int shm_id;

// Function to release shared memory and terminate processes
void releaseSHM(int signum)
{
    int status;

    // Release the shared memory
    status = shmctl(shm_id, IPC_RMID, NULL);
    if (status == 0)
    {
        printf("Shared memory with id %d successfully released\n", shm_id);
    }
    else if (status == -1)
    {
        printf("Error releasing shared memory with id %d\n", shm_id);
    }
    else
    {
        printf("Unknown error releasing shared memory with id %d\n", shm_id);
    }

    // Terminate all processes
    status = kill(0, SIGKILL);
    if (status == 0)
    {
        printf("All processes successfully terminated\n");
    }
    else if (status == -1)
    {
        perror("Kill failed");
        printf("Error terminating all processes\n");
    }
    else
    {
        printf("Unknown error terminating all processes\n");
    }
}

// This two functions are used to synchronize the parent and child processes along with pause(), kill() and sleep() functions
/*  pause() causes the calling process (or thread) to sleep until a
        signal is delivered that either terminates the process or causes
        the invocation of a signal-catching function. -- from man 2 pause

    So I write two functions to handle the signals, These functions does nothing other than
        causing the child/parent process to wake up from pause() and continue the execution
       */
// This signal handler is used to wake up the child process from pause()
void childHandler(int signum)
{
    // Child process received signal SIGUSR1 to continue execution
}

// This signal handler is used to wake up the parent process from pause()
void parentHandler(int signum)
{
    // Parent process received signal SIGUSR2 to continue execution
}

// Recursive function to calculate factorial
long int factorial(int n)
{
    if (n == 0)
    {
        return 1;
    }
    else
    {
        return n * factorial(n - 1);
    }
}

int main()
{
    int status, counter = 1;
    pid_t pidFork, pidWait;
    sighandler_t shandler;

    // Set up signal handler for releasing shared memory and terminating processes
    shandler = signal(SIGINT, releaseSHM);
    if (shandler == SIG_ERR)
    {
        perror("Signal handler error");
        exit(1);
    }

    // Create shared memory
    shm_id = shmget(IPC_PRIVATE, sizeof(long int), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("Shared memory error");
        exit(1);
    }
    else
    {
        printf("Shared memory with id %d successfully created\n", shm_id);
    }

    // Fork a child process
    pidFork = fork();
    if (pidFork == -1)
    {
        perror("Fork error");
        exit(1);
    }
    else if (pidFork == 0) // Child Process
    {
        long int *shm_ptr;
        // Attach the shared memory
        shm_ptr = (long int *)shmat(shm_id, NULL, 0);
        if (shm_ptr == (long int *)-1)
        {
            perror("Shared memory attachment error");
            exit(1);
        }

        // Start an infinite loop in the child process
        // Wait for the signal to proceed
        sighandler_t chandler;
        // Signal handler for child process - wakes up the child process from pause() in the main loop
        chandler = signal(SIGUSR1, childHandler);
        if (chandler == SIG_ERR)
        {
            perror("Signal handler error");
            exit(1);
        }
        while (1)
        {

            pause();

            // Read from shared memory and store in a variable
            int n = (int)*shm_ptr;
            // Calculate the factorial
            printf("Child process read number: %d from the shared memory.\n", n);
            long int result = factorial(n);
            // Write the result to shared memory
            *shm_ptr = result;
            printf("Child process calculated the factorial of %d and wrote it in the shared memory\n", n);
            // sleep(1);

            // Notify the parent process to continue
            kill(getppid(), SIGUSR2);
        }
    }
    else // Parent Process
    {
        long int *shm_ptr;
        // Attach the shared memory
        shm_ptr = (long int *)shmat(shm_id, NULL, 0);
        if (shm_ptr == (long int *)-1)
        {
            perror("Shared memory attachment error");
            exit(1);
        }

        // Wait for the signal from the child process
        sighandler_t phandler;
        // Signal handler for parent process - wakes up the parent process from pause() in the main loop
        phandler = signal(SIGUSR2, parentHandler);
        // Start an infinite loop in the parent process
        while (1)
        {
            int n;
            // Generating a random number between 1 and 20 for the parent process
            n = rand() % 20 + 1; // Restricting to numbers <= 20 due to long int limitations
            // Write to shared memory
            *shm_ptr = (long int)n;
            printf("Iteration No: %d\n", counter);
            printf("Parent process generated number: %d, and wrote it in the shared memory\n", n);
            sleep(1);

            // Notify the child process to continue
            kill(pidFork, SIGUSR1);

            if (phandler == SIG_ERR)
            {
                perror("Signal handler error");
                exit(1);
            }
            pause();

            // Read from shared memory and store in a variable
            long int result = *shm_ptr;
            printf("Parent process read the factorial of %d from the shared memory.\n", n);
            // Display the result of the factorial calculation
            printf("The factorial of %d is:\t%ld\n\n", n, result);
            counter++;
        }
    }

    // Wait for the child process to finish
    pidWait = wait(&status);
    printf("Child process %d finished with status %d\n", pidFork, status);

    return 0;
}
