#include "mockFile.hpp"
#include <unistd.h>
#include <algorithm>

mockFile::mockFile(size_t fileSize)
{
    fakeFile = new char[fileSize];
    this->fileSize = fileSize;
    position = 0;
}

mockFile::~mockFile()
{
    delete [] fakeFile;
}

int mockFile::mockRead(int fd, void *buffer, size_t n)
{
    std::copy(fakeFile + position, fakeFile + position + n, static_cast<char*>(buffer));
    return 0;
}

int mockFile::mockWrite(int fd, const void *buffer, size_t n)
{
    const char *charBuf = static_cast<const char*>(buffer);
    std::copy(charBuf, charBuf + n, fakeFile + position);
    return 0;
}

off_t mockFile::mockLseek(int fd, off_t offset, int whence)
{
    if (whence == SEEK_SET)
        position = offset;
    if (whence == SEEK_CUR)
        position = position + offset;
    else
        return -1;
    return position;
}

