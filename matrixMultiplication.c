#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 10

// function to print the matrix
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

// function to generate random elements for the matrix
void generateRandomElements(int matrix[][MAX_SIZE], int row, int column)
{
    srand(time(0)); // seed for the random number generator
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            matrix[i][j] = rand() % 6; // random numbers between 0 and 5
        }
    }
}

// function to get matrix elements entered by the user

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

// function to multiply two matrices
void multiplyMatrices(int firstMatrix[][MAX_SIZE], int secondMatrix[][MAX_SIZE], int resultMatrix[][MAX_SIZE], int r1, int c1, int r2, int c2)
{
    for (int i = 0; i < r1; i++)
    {
        for (int j = 0; j < c2; j++)
        {
            resultMatrix[i][j] = 0;
            for (int k = 0; k < c1; k++)
            {
                resultMatrix[i][j] += firstMatrix[i][k] * secondMatrix[k][j];
            }
        }
    }
}

int main()
{
    int firstMatrix[MAX_SIZE][MAX_SIZE], secondMatrix[MAX_SIZE][MAX_SIZE], resultMatrix[MAX_SIZE][MAX_SIZE];
    int r1, c1, r2, c2, i, j, k, choice;

    printf("Enter rows and columns for first matrix: ");
    scanf("%d %d", &r1, &c1);

    printf("Enter rows and columns for second matrix: ");
    scanf("%d %d", &r2, &c2);

    // check if the matrices can be multiplied
    if (c1 != r2)
    {
        printf("The matrices can't be multiplied with each other.\n");
        return 1;
    }

    // ask if the user wants to generate random elements or enter them manually
    printf("Do you want to enter elements (1) or generate random elements (0)? (1/0): ");
    scanf("%d", &choice);
    if (choice == 0)
    {
        generateRandomElements(firstMatrix, r1, c1);
        generateRandomElements(secondMatrix, r2, c2);
    }
    else
    {
        printf("Enter elements for first matrix: \n");
        getMatrixElements(firstMatrix, r1, c1);
        printf("Enter elements for second matrix: \n");
        getMatrixElements(secondMatrix, r2, c2);
    }

    printf("First matrix: \n");
    printMatrix(firstMatrix, r1, c1);
    printf("Second matrix: \n");
    printMatrix(secondMatrix, r2, c2);

    multiplyMatrices(firstMatrix, secondMatrix, resultMatrix, r1, c1, r2, c2);
    printf("Resultant matrix: \n");
    printMatrix(resultMatrix, r1, c2);
}