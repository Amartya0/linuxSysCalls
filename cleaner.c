// a program to delete all file with .out, starting with test extension in the current directory

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char command[100];
    // todo: add flag wise deletion for different file types
    strcpy(command, "rm -I *.out");
    system(command);
    strcpy(command, "rm -I test*");
    system(command);
    return 0;
}