#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <vector>

class mockFile{
public:
    int mockRead(int fd, void *buffer, size_t n);
    int mockWrite(int fd, const void *buffer, size_t n);
    off_t mockLseek(int fd, off_t offset, int whence);
    mockFile(size_t fileSize);
    ~mockFile();
private:
    char *fakeFile;
    int fileSize;
    int position;
};