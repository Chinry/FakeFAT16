#include <unistd.h>

int ff_read(int fd, void *buffer, size_t n);
int ff_write(int fd, const void *buffer, size_t n);
off_t ff_lseek(int fd, off_t offset, int whence);