#ifdef __cplusplus
extern "C" {
#endif

#include "ff_actions.h"
#include <stdio.h>
#include <errno.h>

static readFuncPtr readFunc;
static writeFuncPtr writeFunc;
static lseekFuncPtr lseekFunc;

void ff_setFuncs(readFuncPtr read, writeFuncPtr write, lseekFuncPtr lseek)
{
    readFunc = read;
    writeFunc = write;
    lseekFunc = lseek;
}

int ff_read(int fd, void *buffer, size_t n)
{
    return readFunc(fd, buffer, n);
}

int ff_write(int fd, const void *buffer, size_t n)
{
    return writeFunc(fd, buffer, n);
}

off_t ff_lseek(int fd, off_t offset, int whence)
{
    return lseekFunc(fd, offset, whence);
}

int machine_read(int fd, void *buffer, size_t n)
{
    int size = read(fd, buffer, n);
    if (size < 0 || (size_t) size != n)
    {
        printf("ERROR read at %li failed. Read %i bytes\n", ff_lseek(fd, 0, SEEK_CUR), size);
        perror("error code");
    }
    return size;
}

int machine_write(int fd, const void *buffer, size_t n)
{
    int size = write(fd, buffer, n);
    if (size < 0 || (size_t) size != n)
    {
        printf("ERROR write at %li failed\n", ff_lseek(fd, 0, SEEK_CUR));
        perror("error code");
    }
    return size;
}

off_t machine_lseek(int fd, off_t offset, int whence)
{
    off_t loc = lseek(fd, offset, whence);
    if (loc < 0)
    {
        printf("failed to seek %li\n", offset);
        perror("ERROR lseek");
    }
    return loc;
}

void ff_init(){
    readFunc = machine_read;
    writeFunc = machine_write;
    lseekFunc = machine_lseek;
}

#ifdef __cplusplus
}
#endif