#ifdef __cplusplus
extern "C" {
#endif

#include "dirEntry.h"
#include "ff_actions.h"


static void extractNameAndExt(const char *filename, char *name, char *ext)
{
    memset(name, ' ', 8);
    int i = 0;
    while (i < 8)
    {
        if(filename[i] == '.')
        {
            memcpy(ext, filename + i + 1, 3);
            return;
        }
        name[i] = filename[i];
        i++;
        
    }
    memset(ext, 0, 3);
    return;
}

void writeDirEntry(int fd, dirEntry *entry, int offset)
{
    ff_lseek(fd, offset, SEEK_SET);
    ff_write(fd, entry->filename, 8);
    ff_write(fd, entry->extension, 3);
    ff_write(fd, &entry->attributes, 1);
    ff_lseek(fd, offset + 20, SEEK_SET);
    ff_write(fd, &entry->firstLogicalCluster, 2);
    ff_write(fd, &entry->fileSize, 4);
}

dirEntry readDirEntry(int fd, size_t offset)
{
    dirEntry entry;
    uint8_t buffer[DIR_ENTRY_SIZE];
    ff_lseek(fd, offset, SEEK_SET);
    ff_read(fd, buffer, DIR_ENTRY_SIZE);
    memcpy(&entry.filename, buffer, 8 * sizeof(char));
    memcpy(&entry.extension, buffer + 8, 3 * sizeof(char));
    if(!strncmp(entry.extension, "   ", 3))
    {
        memset(entry.extension, 0, 3);
    } 
    memcpy(&entry.attributes, buffer + 11, sizeof(uint8_t));
    memcpy(&entry.firstLogicalCluster, buffer + 26, sizeof(uint16_t));
    memcpy(&entry.fileSize, buffer + 28, sizeof(uint32_t));
    return entry;
}

int findFreeDirEntry(int fd, int offset)
{
    ff_lseek(fd, offset, SEEK_SET);
    char buffer[8];
    do{
        ff_read(fd, buffer, 8);
        ff_lseek(fd, 24, SEEK_CUR);;
    }while(buffer[0] != 0);
    return ff_lseek(fd, 0, SEEK_CUR) - DIR_ENTRY_SIZE;
}

openDir findDirEntry(int fd, const char *path)
{
    openDir location;
    dirEntry entry;
    char name[9];
    char ext[4];

    name[8] = 0;
    ext[3] = 0;
    extractNameAndExt(path, name, ext);

    for (int i = 0; i < SECTOR; i+=32)
    {
        entry = readDirEntry(fd, ROOT + i);
        if(!strncmp(name, entry.filename, 8) && !strncmp(ext, entry.extension, 3))
        {
            location.dirLoc = ROOT;
            location.entryLoc = i;
            return location;
        }
    }

    location.dirLoc = -1;
    location.entryLoc = -1;
    return location;
}


#ifdef __cplusplus
}
#endif
