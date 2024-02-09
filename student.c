// header file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>

// store a filepath to use as key
#define KEY_FILE_PATH "/home/kartos/work/linuxSysCalls/dummy_key.txt"

// define a structure to hold students' information same as in teacher.c
typedef struct
{
    int roll;
    time_t timestamp;
} student_info;

// main function
void main(int argc, char *argv[])
{

    int index_of_student, shm_id;
    key_t shm_key;
    student_info *student;
    // check if two arguments are passed
    if (argc != 2)
    {
        printf("Usage: %s <roll>\n", argv[0]);
        exit(1);
    }
    // get the same key as teacher.c
    shm_key = ftok(KEY_FILE_PATH, 1);
    if (shm_key == -1)
    {
        perror("ftok");
        exit(1);
    }
    // get the shared memory id
    shm_id = shmget(shm_key, 0, 0);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(1);
    }
    // attach the shared memory to the process
    student = (student_info *)shmat(shm_id, NULL, 0);
    if (student == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }
    index_of_student = atoi(argv[1]);
    // set the roll from command line argument and timestamp as the current time from system
    // Here index is the value provided in argv[1]-1, this is because loop index is in 0 to (n-1)range, but we expect students' roll number to be in 1-n range
    // that is student 1's info is saved at index 0, 2's at index 1...., n's at index n-1

    // check if for this student information is already there
    if (student[index_of_student - 1].roll != -1)
    {
        printf("Student with roll number %d already logged attendance at %s.", index_of_student, ctime(&student[index_of_student - 1].timestamp));
    }
    else
    {
        student[index_of_student - 1].roll = atoi(argv[1]);
        student[index_of_student - 1].timestamp = time(NULL);
    }

    // detach the shared memory from the process
    if (shmdt(student) == -1)
    {
        perror("shmdt");
        exit(1);
    }
    return;
}
