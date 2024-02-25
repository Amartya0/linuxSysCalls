// header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
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

    // check if two arguments are passed
    if (argc != 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 0;
    }
    // create argv[1] number of child processes
    for (int i = 1; i < atoi(argv[1]) + 1; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            printf("Child process %d\n", i);
            printf("Factorial of %d is %d\n", i, factorial(i));
            exit(0);
        }
        else
        {
            // Parent process
            wait(NULL);
        }
    }

    return 0;
}