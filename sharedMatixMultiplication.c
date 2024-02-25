#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_Rows 50          // maximum number of rows
#define MAX_Columns 50       // maximum number of columns
#define MAX_Random_Value 100 // maximum value for random elements

typedef int matrix[MAX_Rows][MAX_Columns]; // Defining a type for a matrix
typedef int row[MAX_Columns];              // Defining a type for a row (1D array)

// Function to print the elements of a matrix
void printMatrix(matrix matrix, int rows, int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (j != 0)
            {
                printf("\t%d", matrix[i][j]);
            }
            else
            {
                printf("%d", matrix[i][j]);
            }
        }
        printf("\n\n");
    }
}

// Function to generate random elements for a matrix
void generateRandomElements(matrix matrix, int rows, int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            matrix[i][j] = rand() % MAX_Random_Value;
        }
    }
}

// Function to get elements of a matrix from user input
void getElementsFromUser(matrix matrix, int rows, int columns)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            printf("Enter elements[%d][%d]: ", i + 1, j + 1);
            scanf("%d", &matrix[i][j]);
        }
    }
}

int main()
{
    // declare variables
    int r1, c1, r2, c2, i, j, k, choice, shmid, shmdtid, shmctlid;
    matrix matrix1, matrix2;
    pid_t pidFork[MAX_Columns + 1];
    pid_t pidWait[MAX_Columns + 1];
    int status[MAX_Columns + 1];

    // Taking input for the dimensions of the matrices
    printf("Enter rows and columns size for the first matrix (maximum %d*%d): ", MAX_Rows, MAX_Columns);
    scanf("%d %d", &r1, &c1);
    printf("Enter rows and columns size for the second matrix (maximum %d*%d): ", MAX_Rows, MAX_Columns);
    scanf("%d %d", &r2, &c2);

    // Checking if the input dimensions are within bounds and valid for matrix multiplication
    if (r1 > MAX_Rows || c1 > MAX_Columns || r2 > MAX_Rows || c2 > MAX_Columns)
    {
        printf("Entered dimensions are out of bounds\n");
        return 1;
    }
    else if (r1 <= 0 || c1 <= 0 || r2 <= 0 || c2 <= 0)
    {
        printf("Rows and columns cannot be less than or equal to 0\n");
        return 1;
    }
    else if (c1 != r2)
    {
        printf("Invalid input for matrix multiplication\n");
        return 1;
    }

    // Taking input for whether to generate random elements or take input manually
    printf("Enter 1 to generate random elements or 2 to enter elements manually:(1/2)");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
        // Generate random elements for the first matrix
        generateRandomElements(matrix1, r1, c1);
        // Generate random elements for the second matrix
        generateRandomElements(matrix2, r2, c2);
        break;
    case 2:
        // Get elements of the first matrix from user input
        printf("Enter elements for the first matrix: \n");
        getElementsFromUser(matrix1, r1, c1);
        // Get elements of the second matrix from user input
        printf("Enter elements for the second matrix: \n");
        getElementsFromUser(matrix2, r2, c2);
        break;

    default:
        printf("Invalid input\n");
        return 1;
    }
    // Print the first matrix
    printf("First matrix: \n");
    printMatrix(matrix1, r1, c1);
    // Print the second matrix
    printf("Second matrix: \n");
    printMatrix(matrix2, r2, c2);

    // Create shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(int) * r1 * c2, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }

    // Attach shared memory to a 2D array to store the result matrix
    row *resultMatrix = (row *)shmat(shmid, 0, 0);

    // Create the number of child processes equal to the number of rows in the first matrix as each child process will be responsible for a row in the resultant matrix.
    // And we know that the number of rows in the resultant matrix is equal to the number of rows in the first matrix.
    for (i = 0; i < r1; i++)
    {
        pidFork[i] = fork();
        if (pidFork[i] == 0)
        {
            // Calculate the assigned row of the resultant matrix
            for (j = 0; j < c2; j++)
            {
                resultMatrix[i][j] = 0;
                for (k = 0; k < c1; k++)
                {
                    resultMatrix[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
            exit(0);
        }
    }

    // Wait for all child processes to finish
    for (i = 0; i < r1; i++)
    {
        // Here we are waiting for all child processes to finish before proceeding to print the resultant matrix,
        // because we need to ensure that all processes have completed their calculations before accessing the result.
        // If we printed the resultant matrix inside the loop, it would print partial results as soon as each process finished,
        // which may not be the complete result of the matrix multiplication.
        // We are also not concerned with the order in which the child processes finish.
        pidWait[i] = waitpid(pidFork[i], &status[i], 0);
        if (pidWait[i] == -1)
        {
            printf("Error in waiting for child process %d\n", i);
        }
        if (WEXITSTATUS(status[i]) != 0) // WEXITSTATUS returns the exact status of the child process
        {
            printf("Child process %d, id: %d terminated with status %d\nRow %d will be affected.\n", i, pidFork[i], WEXITSTATUS(status[i]), i + 1);
        }
    }

    // Printing the resultant matrix
    printf("Resultant matrix: \n");
    printMatrix(resultMatrix, r1, c2);

    // Detaching and destroying the shared memory
    shmdtid = shmdt(resultMatrix);
    if (shmdtid == -1)
    {
        perror("shmdt");
        return 1;
    }
    shmctlid = shmctl(shmid, IPC_RMID, 0);
    if (shmctlid == -1)
    {
        perror("shmctl");
        return 1;
    }

    return 0;
}
