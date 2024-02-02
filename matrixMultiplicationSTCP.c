#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_SIZE 10

// Function to print the matrix
void printMatrix(int matrix[][MAX_SIZE], int row, int column)
{
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
}

// Function to generate random elements for the matrix
void generateRandomElements(int matrix[][MAX_SIZE], int row, int column)
{
    srand(time(0)); // Seed for the random number generator
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            matrix[i][j] = rand() % 6; // Random numbers between 0 and 5
        }
    }
}

// Function to get matrix elements entered by the user
void getMatrixElements(int matrix[][MAX_SIZE], int row, int column)
{
    printf("Enter elements: \n");

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            printf("Enter a%d%d: ", i + 1, j + 1);
            scanf("%d", &matrix[i][j]);
        }
    }
}

int main()
{
    int firstMatrix[MAX_SIZE][MAX_SIZE], secondMatrix[MAX_SIZE][MAX_SIZE], resultMatrix[MAX_SIZE][MAX_SIZE];
    int r1, c1, r2, c2, i, j, k, choice, status;
    pid_t pidFork, pidWait;

    // Input matrix dimensions
    printf("Enter rows and columns size for the first matrix (maximum 10*10): ");
    scanf("%d %d", &r1, &c1);

    printf("Enter rows and columns size for the second matrix (maximum 10*10): ");
    scanf("%d %d", &r2, &c2);

    // Check if the matrices can be multiplied
    if (c1 != r2)
    {
        printf("The matrices can't be multiplied with each other.\n");
        return 1;
    }
    else if (r1 > MAX_SIZE || c1 > MAX_SIZE || r2 > MAX_SIZE || c2 > MAX_SIZE)
    {
        printf("The maximum size of the matrix is 10.\n");
        return 1;
    }

    // Ask if the user wants to enter elements or generate random elements
    printf("Do you want to enter elements (1) or generate random elements (0)? (1/0): ");
    scanf("%d", &choice);
    if (choice == 0)
    {
        generateRandomElements(firstMatrix, r1, c1);
        generateRandomElements(secondMatrix, r2, c2);
    }
    else
    {
        printf("Enter elements for the first matrix: \n");
        getMatrixElements(firstMatrix, r1, c1);
        printf("Enter elements for the second matrix: \n");
        getMatrixElements(secondMatrix, r2, c2);
    }

    // Display the input matrices
    printf("First matrix: \n");
    printMatrix(firstMatrix, r1, c1);
    printf("Second matrix: \n");
    printMatrix(secondMatrix, r2, c2);

    // Matrix multiplication using fork
    for (int i = 0; i < r1; i++)
    {
        for (int j = 0; j < c2; j++)
        {
            resultMatrix[i][j] = 0;
            pidFork = fork();

            if (pidFork == 0)
            {
                // Child process calculates the element of the result matrix
                for (int k = 0; k < c1; k++)
                {
                    resultMatrix[i][j] += firstMatrix[i][k] * secondMatrix[k][j];
                }
                exit(resultMatrix[i][j]);
            }
            else
            {
                // Parent process waits for the child process to finish
                pidWait = wait(&status);
                if (pidWait == -1)
                {
                    perror("wait");
                    exit(1);
                }

                printf("Child process %d id: %d finished with status %d\n", i * r1 + j + 1, pidFork, status);

                // Adjust status to get the exact value returned by the child process
                if (status > 255)
                {
                    status = status / 256;
                }
                resultMatrix[i][j] = status;
            }
        }
    }

    // Display the resultant matrix
    printf("\nResultant matrix: \n");
    printMatrix(resultMatrix, r1, c2);

    return 0;
}
