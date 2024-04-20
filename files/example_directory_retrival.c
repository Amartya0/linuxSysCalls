#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

void listDir(char *dirName)
{
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[1000];
    dir = opendir(dirName);

    if (dir == NULL)
    {
        perror("Error opening directory");
        exit(1);
    }

    while ((dirEntry = readdir(dir)) != NULL)
    {
        sprintf(name, "%s/%s", dirName, dirEntry->d_name);
        lstat(name, &inode);

        // Test the type of file
        if (S_ISDIR(inode.st_mode))
            printf("dir ");
        else if (S_ISREG(inode.st_mode))
            printf("file ");
        else if (S_ISLNK(inode.st_mode))
            printf("link ");
        else
            printf("unknown ");

        printf("%s\n", dirEntry->d_name);
    }

    closedir(dir);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("USAGE: %s directory_name\n", argv[0]);
        exit(0);
    }

    printf("Contents of the directory:\n");
    listDir(argv[1]);

    return 0;
}
