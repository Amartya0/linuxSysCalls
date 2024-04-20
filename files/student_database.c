
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "student.data"
#define INDEX_FILE "student.index"

struct Student
{
    int roll;
    char first_name[50];
    char middle_name[50];
    char last_name[50];
    char *description;
};

void addStudent(struct Student newStudent)
{
    FILE *dataFile = fopen(DATA_FILE, "ab");
    FILE *indexFile = fopen(INDEX_FILE, "ab");

    if (!dataFile || !indexFile)
    {
        printf("Error: Unable to open files for adding student record.\n");
        return;
    }

    fseek(dataFile, 0, SEEK_END);
    long offset = ftell(dataFile);

    fwrite(&newStudent, sizeof(struct Student), 1, dataFile);
    fwrite(&offset, sizeof(long), 1, indexFile);

    fclose(dataFile);
    fclose(indexFile);
}

void searchStudent(int rollNumber)
{
    FILE *indexFile = fopen(INDEX_FILE, "rb");
    FILE *dataFile = fopen(DATA_FILE, "rb");

    if (!indexFile || !dataFile)
    {
        printf("Error: Unable to open files for searching student record.\n");
        return;
    }

    fseek(indexFile, 0, SEEK_SET);
    fseek(dataFile, 0, SEEK_SET);

    long offset;
    while (fread(&offset, sizeof(long), 1, indexFile) == 1)
    {
        struct Student tempStudent;
        fseek(dataFile, offset, SEEK_SET);
        fread(&tempStudent, sizeof(struct Student), 1, dataFile);

        if (tempStudent.roll == rollNumber)
        {
            printf("Student found:\n");
            printf("Roll: %d\n", tempStudent.roll);
            printf("First Name: %s\n", tempStudent.first_name);
            printf("Middle Name: %s\n", tempStudent.middle_name);
            printf("Last Name: %s\n", tempStudent.last_name);
            printf("Description: %s\n", tempStudent.description);
            fclose(indexFile);
            fclose(dataFile);
            return;
        }
    }

    printf("Student record with roll number %d not found.\n", rollNumber);
    fclose(indexFile);
    fclose(dataFile);
}

void modifyStudent(int rollNumber, struct Student newStudent)
{
    FILE *indexFile = fopen(INDEX_FILE, "rb+");
    FILE *dataFile = fopen(DATA_FILE, "rb+");

    if (!indexFile || !dataFile)
    {
        printf("Error: Unable to open files for modifying student record.\n");
        return;
    }

    fseek(indexFile, 0, SEEK_SET);
    fseek(dataFile, 0, SEEK_SET);

    long offset;
    while (fread(&offset, sizeof(long), 1, indexFile) == 1)
    {
        struct Student tempStudent;
        fseek(dataFile, offset, SEEK_SET);
        fread(&tempStudent, sizeof(struct Student), 1, dataFile);

        if (tempStudent.roll == rollNumber)
        {
            fseek(dataFile, offset, SEEK_SET);
            fwrite(&newStudent, sizeof(struct Student), 1, dataFile);
            fclose(indexFile);
            fclose(dataFile);
            return;
        }
    }

    printf("Student record with roll number %d not found.\n", rollNumber);
    fclose(indexFile);
    fclose(dataFile);
}

void deleteStudent(int rollNumber)
{
    FILE *indexFile = fopen(INDEX_FILE, "rb+");
    FILE *dataFile = fopen(DATA_FILE, "rb+");
    FILE *tempIndexFile = fopen("temp_index.dat", "wb+");

    if (!indexFile || !tempIndexFile)
    {
        printf("Error: Unable to open files for deleting student record.\n");
        return;
    }

    fseek(indexFile, 0, SEEK_SET);

    long offset;
    while (fread(&offset, sizeof(long), 1, indexFile) == 1)
    {
        struct Student tempStudent;
        fseek(dataFile, offset, SEEK_SET);
        fread(&tempStudent, sizeof(struct Student), 1, dataFile);

        if (tempStudent.roll != rollNumber)
        {
            fwrite(&offset, sizeof(long), 1, tempIndexFile);
        }
    }

    fclose(indexFile);
    fclose(tempIndexFile);

    remove(INDEX_FILE);
    rename("temp_index.dat", INDEX_FILE);
}

void compactFiles()
{
    FILE *indexFile = fopen(INDEX_FILE, "rb");
    FILE *tempIndexFile = fopen("temp_index.dat", "wb");
    FILE *dataFile = fopen(DATA_FILE, "rb");
    FILE *tempDataFile = fopen("temp_data.dat", "wb");

    if (!indexFile || !tempIndexFile || !dataFile || !tempDataFile)
    {
        printf("Error: Unable to open files for compacting.\n");
        return;
    }

    fseek(indexFile, 0, SEEK_SET);
    fseek(dataFile, 0, SEEK_SET);

    long offset;
    long newOffset = 0;
    while (fread(&offset, sizeof(long), 1, indexFile) == 1)
    {
        struct Student tempStudent;
        fseek(dataFile, offset, SEEK_SET);
        fread(&tempStudent, sizeof(struct Student), 1, dataFile);

        // Write the updated offset to the temporary index file
        fwrite(&newOffset, sizeof(long), 1, tempIndexFile);

        // Write the student record to the temporary data file
        fwrite(&tempStudent, sizeof(struct Student), 1, tempDataFile);

        // Update the new offset for the next record
        newOffset += sizeof(struct Student);
    }
    printf("Size of files in bytes:\n  Before Compaction: %ld\tAfter Compaction: %ld\n", ftell(dataFile), ftell(tempDataFile));
    fclose(indexFile);
    fclose(tempIndexFile);
    fclose(dataFile);
    fclose(tempDataFile);

    remove(INDEX_FILE);
    remove(DATA_FILE);

    rename("temp_index.dat", INDEX_FILE);
    rename("temp_data.dat", DATA_FILE);
}

void main()
{
    // Adding some sample students
    struct Student student1 = {1, "Amartya", "", "Jash", "Dumb guy with street smartness."};
    struct Student student2 = {2, "Saugata", "", "Choudhury", "I do not know what to write here!"};
    struct Student student3 = {3, "Abhishek", "", "Adhikari", "Smartest person in the room, always!"};

    addStudent(student1);
    addStudent(student2);
    addStudent(student3);

    // Searching for a student
    printf("\nSearching for student with roll number 1:\n");
    searchStudent(1);

    // Modifying a student
    struct Student modifiedStudent = {2, "Saugata", "", "Choudhury", "Still don't know what to write here!"};
    printf("\nModifying student with roll number 2:\n");
    modifyStudent(2, modifiedStudent);

    // Searching for the modified student
    printf("\nSearching for modified student with roll number 2:\n");
    searchStudent(2);

    // Deleting a student
    printf("\nDeleting student with roll number 2:\n");
    deleteStudent(2);

    // Searching for the deleted student
    printf("\nSearching for deleted student with roll number 2:\n");
    searchStudent(2);

    // Compacting files
    printf("\nCompacting files...\n");
    compactFiles();

    // Searching for a student
    printf("\nSearching for student with roll number 3:\n");
    searchStudent(3);

    // Remove the files
    remove(INDEX_FILE);
    remove(DATA_FILE);

    exit(0);
}
