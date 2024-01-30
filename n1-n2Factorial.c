// This program can only calculate factorial of numbers from 0 to 12
// This is because we used int to store the factorial value

// header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// function to calculate factorial
int factorial(int n)
{
    if (n < 0)
    {
        return -1;
    }
    else if (n == 0)
    {
        return 1;
    }
    else if (n == 1)
    {
        return 1;
    }
    else
    {
        return n * factorial(n - 1);
    }
}

// main function
int main(int argc, char *argv[])
{

    // check if three arguments are passed
    if (argc != 3)
    {
        printf("Usage: %s <number1> <number2>\n", argv[0]);
        return 0;
    }

    // create a pipe to communicate between processes
    int fd[2];
    if (pipe(fd) == -1)
    {
        printf("Pipe failed.\n");
        return 0;
    }

    // check if values of n1 and n2 are valid
    int num, n1, n2, temp;
    temp = 0;
    n1 = atoi(argv[1]);
    n2 = atoi(argv[2]);
    num = n2 - n1 + 1;
    if (num < 1)
    {
        printf("Invalid range of numbers.\n");
        return 0;
    }
    else
    {
        // create num number of child processes
        for (int i = 0; i < num; i++)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                // child process
                // check if 1st child process
                if (i == 0)
                {
                    // close read end of pipe
                    close(fd[0]);
                    // calculate factorial using factorial function
                    int fact = factorial(n1);
                    // print the result and write it to pipe
                    printf("From child %d:\t%d! = %d\n", i + n1, i + n1, fact);
                    write(fd[1], &fact, sizeof(int));
                    // close write end of pipe
                    close(fd[1]);
                }
                else if (i == num - 1)
                {
                    // read the previous result from pipe
                    int prev;
                    read(fd[0], &prev, sizeof(int));
                    // close the pipe
                    close(fd[0]);
                    close(fd[1]);
                    // calculate factorial using last result
                    int fact = prev * (i + n1);
                    // print the result
                    printf("From child %d:\t%d! = %d\n", i + n1, i + n1, fact);
                }
                else
                {
                    // read the previous result from pipe
                    int prev;
                    read(fd[0], &prev, sizeof(int));
                    // close read end of pipe
                    close(fd[0]);
                    // calculate factorial using last result
                    int fact = prev * (i + n1);
                    // print the result and write it to pipe
                    printf("From child %d:\t%d! = %d\n", i + n1, i + n1, fact);
                    write(fd[1], &fact, sizeof(int));
                    // close write end of pipe
                    close(fd[1]);
                }
                // terminate child process
                exit(0);
            }
            else
            {
                // parent process
                // wait for child process to terminate
                wait(NULL);
            }
        }
    }

    return 0;
}