// header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// function to calculate x^n where n>=0
int power(int x, int n)
{
    if (n == 0)
    {
        return 1;
    }
    else
    {
        return x * power(x, n - 1);
    }
}

// main function
int main(int argc, char *argv[])
{
    // check if at least two arguments are passed
    if (argc < 3)
    {
        printf("Usage: %s <number1> <number2> <number3> ... <numberN>\nAt least two number is required.\n", argv[0]);
        return 0;
    }

    // get the number of terms in the polynomial
    int n = argc - 2;
    // This n is not the degree rather it is degree+1. For example, if the polynomial is 2x^3+3x^2+4x+5, then n=4.
    // This n is also equal to the number of child processes we need to create.

    // get the value of x
    int x = atoi(argv[1]);

    // First we will check if n=1. If n=1, then we will not create any pipe.
    // we will also not create child processes in loop if n=1.
    // we will just create a single child process and print the argv[2] as the answer.
    // When x = 0, we will do the same thing as apart from the constant term, all the other terms will be zero.
    if (n == 1 | x == 0)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // child process
            printf("Answer: %d\n", atoi(argv[2]));
            exit(0);
        }
        else
        {
            // parent process
            wait(NULL);
        }
    }
    // If n>1, then we will create a pipe and create n numbers of child processes.
    // First child process will calculate the nth term and write it to the pipe.
    // From the second child process to the second last child process, each child process will calculate the (n-i)th term and add it to the result it gets from pipe, and then write the result to pipe.
    // The last child process will calculate the first term and add it to the result it gets from the pipe, and then print the result as the final answer.
    // As we have taken care of the case when n=1, we know that at least two processes will be created, So the first process will always write to pipe and the last process will always read from pipe.
    else
    {

        int fd[2];
        if (pipe(fd) == -1)
        {
            printf("Pipe failed.\n");
            return 0;
        }

        // create n number of child processes
        for (int i = 0; i < n; i++)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                // child process
                if (i == 0)
                {
                    // close the read end of pipe
                    close(fd[0]);
                    // calculate the nth term
                    int a = atoi(argv[i + 2]);
                    int result = a * power(x, n - i - 1);
                    // write the nth term to pipe.
                    write(fd[1], &result, sizeof(int));
                    // close the write end of pipe
                    close(fd[1]);
                    exit(0);
                }
                else if (i == n - 1)
                {
                    // close the write end of pipe
                    close(fd[1]);
                    // calculate the 0th term
                    int a = atoi(argv[i + 2]);
                    int result = a * power(x, n - i - 1);
                    // get the summation of all the previous terms from pipe
                    int prev;
                    read(fd[0], &prev, sizeof(int));
                    // close the read end of pipe
                    close(fd[0]);
                    // Print the summation of all the prev from pipe + last term as the final result
                    printf("Answer: %d\n", result + prev);
                    exit(0);
                }
                else
                {
                    // calculate the (n-i)th term
                    int a = atoi(argv[i + 2]);
                    int result = a * power(x, n - i - 1);
                    // get the summation of all the previous terms from pipe
                    long int prev;
                    read(fd[0], &prev, sizeof(int));
                    // close the read end of pipe
                    close(fd[0]);
                    // add the result to the summation of all the prev from pipe
                    result += prev;
                    // write the current summation to pipe
                    write(fd[1], &result, sizeof(int));
                    // close the write end of pipe
                    close(fd[1]);
                    exit(0);
                }
            }
            else
            {
                // parent process
                // wait for child process to finish
                wait(NULL);
            }
        }
    }
}
