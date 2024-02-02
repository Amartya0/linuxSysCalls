#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    // check if at least two arguments are passed
    if (argc < 2)
    {
        printf("Usage: %s <fp1> <arg1> <arg2>...<argm> -f <fp2> <arg1> <arg2>...<argk> -f... <fpn> <arg> <arg2>...<argl>\ni.e. Executable file paths followed by their arguments, separated by -f\nAt least one filepath is required.\n", argv[0]);
        return 1;
    }

    // check if -f flag is present at the end also, if not add it
    if (strcmp(argv[argc - 1], "-f") != 0)
    {

        argv[argc] = "-f";
        argc++;
    }

    // count the number of filepaths
    int filepaths = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            filepaths++;
        }
    }

    // Array to store the filepaths and their arguments
    char *filepathsArray[argc - filepaths];
    int filepathsArrayColumnIndex = 0;
    int statusExe, statusWait;
    pid_t pidFork, pidWait;
    int childProcessCounter = 1;

    for (int i = 1; i < argc + 1; i++)
    {
        // If the current argument is the -f flag, execute the specified command along with its arguments in new child process
        if (strcmp(argv[i], "-f") == 0)
        {

            // get the filepaths from 0th index and store in a temporary variable
            char *tempFilepath = filepathsArray[0];
            // get the executable name from the last part of the file path
            filepathsArray[0] = strrchr(tempFilepath, '/');

            // fork a child process
            pidFork = fork();
            if (pidFork == 0)
            {
                // Child process
                printf("----------------%d----------------\n", childProcessCounter);
                // Execute the specified executable file with its arguments
                statusExe = execve(tempFilepath, filepathsArray, NULL);
                if (statusExe == -1)
                {
                    perror("Exec Fails: ");
                }
                exit(1);
            }
            else
            {
                // Parent process
                // wait for the child process to finish

                pidWait = wait(&statusWait);
                if (pidWait == -1)
                {
                    perror("wait");
                    exit(1);
                }
                if (statusWait > 255) // This is just to make the status exactly the same as the one set by the child process
                {
                    statusWait = statusWait / 256;
                }
                printf("pid = %d status = %d!\n", pidWait, statusWait);

                childProcessCounter++; // This is just to keep track of the number of child processes created

                // clear the filepathsArray before the next iteration
                for (int i = 0; i < argc - filepaths; i++)
                {
                    filepathsArray[i] = NULL;
                }
            }

            filepathsArrayColumnIndex = 0;
        }
        else
        {
            // Populate the filepathsArray with file paths and arguments
            filepathsArray[filepathsArrayColumnIndex] = argv[i];
            filepathsArrayColumnIndex++;
        }
    }

    return 0;
}
