#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <sys/types.h>

typedef int (*readFuncPtr)(int, void*, size_t);
typedef int (*writeFuncPtr)(int, const void*, size_t);
typedef off_t (*lseekFuncPtr)(int, off_t, int);

int ff_read(int fd, void *buffer, size_t n);
int ff_write(int fd, const void *buffer, size_t n);
off_t ff_lseek(int fd, off_t offset, int whence);
void ff_setFuncs(readFuncPtr read, writeFuncPtr write, lseekFuncPtr lseek);
void ff_init(void);

#ifdef __cplusplus
}
#endif