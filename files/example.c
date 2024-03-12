#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char buf1[] = "LAB ";
char buf2[] = "OS Linux";

int main(void)
{
    int fd;

    // Creating the file "file.gol"
    if ((fd = creat("file.gol", 0666)) < 0)
    {
        perror("Creation error");
        exit(1);
    }

    // Writing buf1 to the file
    if (write(fd, buf1, sizeof(buf1)) < 0)
    {
        perror("Writing error");
        exit(2);
    }

    // Moving the file pointer to 4096 bytes (4K) from the beginning
    if (lseek(fd, 4096, SEEK_SET) < 0)
    {
        perror("Positioning error");
        exit(3);
    }

    // Writing buf2 to the file
    if (write(fd, buf2, sizeof(buf2)) < 0)
    {
        perror("Writing error");
        exit(2);
    }

    close(fd); // Closing the file
    return 0;
}