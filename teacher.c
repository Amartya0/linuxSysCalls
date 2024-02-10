// header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>

// store a filepath to use as key
#define KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_shared_memory_key.txt"

// define a structure to hold students' information
typedef struct
{
    int roll;
    time_t timestamp;
} student_info;

// define a data type for signal Handler
typedef void (*sighandler_t)(int);

// global variables
int shm_id;
student_info *student;
int number_of_students;

// function to handle signal raised by Ctrl+c key pair
// I am also printing the students' information from this function
// Also the shared memory gets destroyed here
void releaseSHM(int signum)
{
    // local variables
    int status;

    // First we will sort the students' information by the values in their respective timestamp in chronological order
    // I am using bubble sort here, as sorting efficiency is not our target and bubble sort is easy to implement
    for (int i = 0; i < number_of_students; i++)
    {
        // skip the students whose roll no is still -1; i.e. they did not give their attendance
        if (student[i].roll == -1)
        {
            continue;
        }
        for (int j = i + 1; j < number_of_students; j++)
        {
            // again we will skip roll numbers =-1
            if (student[j].roll == -1)
            {
                continue;
            }
            // Here is the bubble swap operation
            if (student[i].timestamp > student[j].timestamp)
            {
                student_info temp = student[i];
                student[i] = student[j];
                student[j] = temp;
            }
        }
    }

    // Now the valid timestamps are in order, but where roll no = -1 null/garbage values are still there
    // So we will print skipping roll=-1, and we will also format the timestamp value to day mon dd hh:mm:ss year format, which is default ctime format
    for (int i = 0; i < number_of_students; i++)
    {
        if (student[i].roll == -1)
        {
            continue;
        }
        if (i == 0)
        {
            printf("\nStudents' timestamps in chronological order:\n");
        }
        printf("Roll:  %d\tTimestamp:  %s", student[i].roll, ctime(&student[i].timestamp));
    }

    // Release the shared memory
    status = shmctl(shm_id, IPC_RMID, NULL);
    if (status == 0)
    {
        printf("Shared memory with id %d successfully released\n", shm_id);
    }
    else
    {
        perror("shmctl failed");
    }

    // Terminate the process
    status = kill(0, SIGKILL);
    if (status == 0)
    {
        // This is only written for reading purpose, if the process was killed successfully this part will never execute
        printf("All processes successfully terminated\n");
    }
    else
    {
        perror("kill failed");
    }
}

int main(int argc, char *argv[])
{
    // Check if exactly two arguments are passed
    if (argc != 2)
    {
        printf("Usage: %s <number_of_students>\n", argv[0]);
        return 1;
    }

    // store the number of students in the global variable
    // atoi() function in used to convert argument into integer
    number_of_students = atoi(argv[1]);

    key_t shm_key;
    sighandler_t shandler;

    // Create a key for the shared memory using the file specified in the macro
    // And store the key in the global variable
    shm_key = ftok(KEY_FILE_PATH, 1);
    if (shm_key == -1)
    {
        perror("ftok failed");
        return 1;
    }

    // Now create a shared memory using this key
    shm_id = shmget(shm_key, sizeof(student_info) * atoi(argv[1]), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("shmget failed");
        return 1;
    }

    // assign the shared memory to the student_info pointer
    student = (student_info *)shmat(shm_id, NULL, 0);
    if (student == (student_info *)-1)
    {
        perror("shmat failed");
        return 1;
    }

    // Set up signal handler for releasing shared memory and terminating processes
    shandler = signal(SIGINT, releaseSHM);
    if (shandler == SIG_ERR)
    {
        perror("Signal handler error");
        return 1;
    }

    // initialize all the students' roll numbers to -1
    for (int i = 0; i < atoi(argv[1]); i++)
    {
        student[i].roll = -1;
    }
    // Now as the output printing and shared memory handling all done by the releaseSHM function nothing more to be done in teacher process
    // We just need to keep this alive until Ctrl+c key pair is pressed
    pause();
    // This part should never execute because when pause breaks releaseSHM also gets executed. So the process will get terminated there.
    // We are just writing this for error checking purposes

    printf("Unknown error occurred!\n");
    return 1;
}