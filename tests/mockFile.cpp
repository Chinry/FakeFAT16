#include "mockFile.hpp"

mockFile::mockFile(size_t fileSize)
{
    fakeFile = new char[fileSize];
    position = 0;
}

int mockFile::mockRead(int fd, void *buffer, size_t n)
{

}

int mockFile::mockWrite(int fd, const void *buffer, size_t n)
{

}

off_t mockFile::mockLseek(int fd, off_t offset, int whence)
{

}