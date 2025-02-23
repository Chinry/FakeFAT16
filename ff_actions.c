#include "ff_actions.h"
#include <stdio.h>
#include <errno.h>

int ff_read(int fd, void *buffer, size_t n)
{
    int size = read(fd, buffer, n);
    if (size < 0 || size != n)
    {
        printf("ERROR read at %li failed. Read %i bytes\n", ff_lseek(fd, 0, SEEK_CUR), size);
        perror("error code");
    }
    return size;
}
int ff_write(int fd, const void *buffer, size_t n)
{
    int size = write(fd, buffer, n);
    if (size < 0 || size != n)
    {
        printf("ERROR write at %li failed\n", ff_lseek(fd, 0, SEEK_CUR));
        perror("error code");
    }
    return size;
}
off_t ff_lseek(int fd, off_t offset, int whence)
{
    off_t loc = lseek(fd, offset, whence);
    if (loc < 0)
    {
        perror("ERROR lseek");
    }
    return loc;
}

