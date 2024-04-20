#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFSIZE 512

int main(int argc, char **argv)
{
    int from, to, nr, nw, n;
    char buf[BUFSIZE];

    // Open the source file for reading
    if ((from = open(argv[1], O_RDONLY)) < 0)
    {
        perror("Error opening source file");
        exit(1);
    }

    // Create or open the destination file for writing
    if ((to = creat(argv[2], 0666)) < 0)
    {
        perror("Error creating destination file");
        exit(2);
    }

    // Read from the source file and write to the destination file
    while ((nr = read(from, buf, sizeof(buf))) != 0)
    {
        if (nr < 0)
        {
            perror("Error reading source file");
            exit(3);
        }
        nw = 0;
        do
        {
            // Write to the destination file
            if ((n = write(to, &buf[nw], nr - nw)) < 0)
            {
                perror("Error writing destination file");
                exit(4);
            }
            nw += n;
        } while (nw < nr);
    }

    // Close both files
    close(from);
    close(to);
}
