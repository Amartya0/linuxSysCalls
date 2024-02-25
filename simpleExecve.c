#include <stdio.h>  /* needed also for perror() */
#include <errno.h>  /* needed for perror() */
#include <unistd.h> /* needed for execve() */

int main(int argc, char *argv[])
{

    int status;
    argv[0] = "ls";
    // An interesting observation: This program will still work if we do not update the argv[0] value

    status = execve("/bin/ls", argv, NULL);
    // this execve() call will replace the currently running process
    // with the process specified in the first argument
    // the second argument is an array of strings that will be passed
    // to the new process as its argv[] array
    // The first element of this array is the name of the program to be executed
    // We know that the "nFactorial.out" program expects one argument
    // So we take only one argument along with the program name from the command line and pass it to the execve() call

    // However, for this to work we also need to change the access of the file nFactorial.out
    // This is done using the chmod command in a terminal

    // kartos@kartos-Alpha-15-B5EEK:~/work/linuxSysCalls$ chmod 777 ./nFactorial.out
    // kartos@kartos-Alpha-15-B5EEK:~/work/linuxSysCalls$ ls -l | grep "nFactorial.out"
    //-rwxrwxrwx 1 kartos kartos 16168 Jan 31 20:23 nFactorial.out

    if (status == -1)
    {
        perror("Exec Fails: ");
    }
}
