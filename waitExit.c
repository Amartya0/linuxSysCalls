#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int status;
    pid_t pid_wait, pid_fork;

    printf("Before fork!\n");

    pid_fork = fork();
    if (pid_fork == 0)
    {
        /* This part is executed by the child process */

        // int i;

        /* The following part is just to keep the child executing something
            so that it is live for some period and does not terminate */

        // for (i = 0; i < 5; i++)
        // {
        //     printf("%d\n", i++);
        //     getchar(); /* Child process waits for user input, remains alive, and does not terminate */
        // }

        getchar(); // I just want to keep the child process alive for one user input for the sake of simplicity
        exit(12);  /* The child process terminates with status 12. Note that 12 is just a number,
                      this status (12) is communicated to the parent process (in some form),
                      and the parent process may use it through wait() if it wishes to do so.
                  */
    }

    /* This part will be executed only by the parent process */
    pid_wait = wait(&status);
    if (pid_wait == -1)
    {
        perror("wait");
        exit(1);
    }
    if (status > 255) // This is just to make the status exactly the same as the one set by the child process
    {
        status = status / 256;
    }
    printf("pid_wait = %d status = %d!\n", pid_wait, status);
    /* The parent process prints the PID of the terminated child (pid_wait)
       and the exit status of the child process (status).
    */
}
