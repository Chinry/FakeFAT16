#include "mockFile.hpp"
#include <unistd.h>
#include <algorithm>

char *fakeFile;
int fileSize;
int position;

void mockFile_init(size_t size)
{
    fakeFile = new char[fileSize];
    fileSize = size;
    position = 0;
}

void mockFile_close()
{
    delete [] fakeFile;
}

int mockFile_mockRead(int fd, void *buffer, size_t n)
{
    std::copy(fakeFile + position, fakeFile + position + n, static_cast<char*>(buffer));
    position += n;
    return 0;
}

int mockFile_mockWrite(int fd, const void *buffer, size_t n)
{
    const char *charBuf = static_cast<const char*>(buffer);
    std::copy(charBuf, charBuf + n, fakeFile + position);
    position += n;
    return 0;
}

off_t mockFile_mockLseek(int fd, off_t offset, int whence)
{
    if (whence == SEEK_SET)
        position = offset;
    if (whence == SEEK_CUR)
        position = position + offset;
    else
        return -1;
    return position;
}

