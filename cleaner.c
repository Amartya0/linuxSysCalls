// a program to delete all file with .out, starting with test extension in the current directory

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc == 1)
    {

        printf("Usage: cleaner [options]\n");
        printf("Options:\n");
        printf("  -h, --help\t\t\tDisplay this information\n");
        printf("  -d, --delete\t\t\tDelete all files with .out extension\n");
        printf("  -t, --test\t\t\tDelete all files starting with test\n");
        printf("  -a, --all\t\t\tDelete all files with .out and starting with test\n");
        return 0;
    }
    if (argc > 2)
    {
        printf("Error: Too many arguments\n");
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        printf("Usage: cleaner [options]\n");
        printf("Options:\n");
        printf("  -h, --help\t\t\tDisplay this information\n");
        printf("  -d, --delete\t\t\tDelete all files with .out extension\n");
        printf("  -t, --test\t\t\tDelete all files starting with test\n");
        printf("  -a, --all\t\t\tDelete all files with .out and starting with test\n");
        return 0;
    }
    else if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--delete") == 0)
    {
        char command[100];
        strcpy(command, "rm -I *.out");
        system(command);
        return 0;
    }
    else if (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--test") == 0)
    {
        char command[100];
        strcpy(command, "rm -I test*");
        system(command);
        return 0;
    }
    else if (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--all") == 0)
    {
        char command[100];
        strcpy(command, "rm -f *.out");
        system(command);
        strcpy(command, "rm -f test*");
        system(command);
        return 0;
    }
    else
    {
        printf("Error: Invalid argument\n");
        return 1;
    }
    return 0;
}