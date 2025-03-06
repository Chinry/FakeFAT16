#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <vector>

int mockFile_mockRead(int fd, void *buffer, size_t n);
int mockFile_mockWrite(int fd, const void *buffer, size_t n);
off_t mockFile_mockLseek(int fd, off_t offset, int whence);
void mockFile_init(size_t size);
void mockFile_close();
