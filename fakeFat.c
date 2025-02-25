#ifdef __cplusplus
extern "C" {
#endif

#include "fakeFat.h"
#include "ff_actions.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define SECTOR 512
#define FAT1 1 * SECTOR
#define FAT2 10 * SECTOR
#define ROOT 19 * SECTOR
#define DATA 33 * SECTOR

#define DISK_SIZE 64 * 1024
#define DIR_ENTRY_SIZE 32

#define CLUSTER_IN_USE 0xFFFF
#define ATTR_DIRECTORY 0x10

#define FILE_OPEN_LIMIT 64

static int fd;

typedef struct
{
    int dirLoc;
    int entryLoc;
} openDir;

static openDir openDirs[FILE_OPEN_LIMIT];

typedef struct
{
    char filename[9];
    char extension[4];
    uint32_t fileSize;
    uint16_t firstLogicalCluster;
    uint8_t attributes;
} dirEntry;

static int findNextFreeCluster()
{
    uint16_t buffer;
    ff_lseek(fd, FAT1 + 2 * sizeof(uint16_t), SEEK_SET);
    do{
        ff_read(fd, &buffer, sizeof(uint16_t));
    }while(buffer != 0);
    return (ff_lseek(fd, 0, SEEK_CUR) - (FAT1 + sizeof(uint16_t))) / 2;
}

static void writeDirEntry(dirEntry *entry, int offset)
{
    ff_lseek(fd, offset, SEEK_SET);
    ff_write(fd, entry->filename, 8);
    ff_write(fd, entry->extension, 3);
    ff_write(fd, &entry->attributes, 1);
    ff_lseek(fd, offset + 20, SEEK_SET);
    ff_write(fd, &entry->firstLogicalCluster, 2);
    ff_write(fd, &entry->fileSize, 4);
}

static dirEntry readDirEntry(size_t offset)
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

void FAT_init()
{
    ff_init();
    uint8_t buf[DISK_SIZE];
    fd = open("storage.dat", O_RDWR|O_CREAT, 777);
    ftruncate(fd, DISK_SIZE);
    memset(buf, 0, DISK_SIZE);
    ff_write(fd, buf, DISK_SIZE);

    uint16_t bytesPerSector = SECTOR;
    uint8_t sectorsPerCluster = 1;
    uint16_t numberOfReservedSectors = 0;
    uint8_t numberOfFATs = 2;
    uint16_t maximumNumberOfRootDirEntries = 0;
    uint16_t totalSectorCount = (DISK_SIZE) / SECTOR;
    uint16_t sectorsPerFAT = 9;
    uint32_t volumeID = 1;
    char fileSystemType[] = "FAT16   ";

    ff_lseek(fd, 11, SEEK_SET);
    ff_write(fd, &bytesPerSector, sizeof(uint16_t));
    ff_write(fd, &sectorsPerCluster, sizeof(uint8_t));
    ff_write(fd, &numberOfReservedSectors, sizeof(uint16_t));
    ff_write(fd, &numberOfFATs, sizeof(uint8_t));
    ff_write(fd, &maximumNumberOfRootDirEntries, sizeof(uint16_t));
    ff_write(fd, &totalSectorCount, sizeof(uint16_t));
    ff_lseek(fd, 1, SEEK_CUR);
    ff_write(fd, &sectorsPerFAT, sizeof(uint16_t));
    ff_lseek(fd, 39, SEEK_SET);
    ff_write(fd, &volumeID, sizeof(uint32_t));
    ff_lseek(fd, 54, SEEK_SET);
    ff_write(fd, fileSystemType, 8);

    memset(openDirs, 0, sizeof(openDir) * FILE_OPEN_LIMIT);

}

void FAT_exit()
{
    fsync(fd);
    close(fd);
}

static int findFreeDirEntry(int offset)
{
    ff_lseek(fd, offset, SEEK_SET);
    char buffer[8];
    do{
        ff_read(fd, buffer, 8);
        ff_lseek(fd, 24, SEEK_CUR);;
    }while(buffer[0] != 0);
    return ff_lseek(fd, 0, SEEK_CUR) - DIR_ENTRY_SIZE;
}

static void setCluster(uint16_t clusterNum, uint16_t set)
{
    ff_lseek(fd, FAT1 + (clusterNum * sizeof(uint16_t)), SEEK_SET);
    ff_write(fd, &set, sizeof(uint16_t));
}

int FAT_mknod(const char *path)
{
    int dirOffset = findFreeDirEntry(ROOT);
    dirEntry entry;
    int i = 0;
    memset(entry.filename, ' ', 8);
    while(path[i] != '.')
    {
        entry.filename[i] = path[i];
        i++;
    }
    i++;
    memcpy(entry.extension, path + i, 3);
    entry.attributes = 0;
    entry.firstLogicalCluster = 0;
    entry.fileSize = 0;
    writeDirEntry(&entry, dirOffset);
    return 0;
}

int FAT_mkdir(const char *path)
{
    int dirOffset = findFreeDirEntry(ROOT);
    dirEntry entry;
    entry.attributes = ATTR_DIRECTORY;
    memset(entry.filename, ' ', 8);
    memcpy(entry.filename, path, strlen(path));
    memset(entry.extension, ' ', 3);
    entry.firstLogicalCluster = findNextFreeCluster();
    writeDirEntry(&entry, dirOffset);
    setCluster(entry.firstLogicalCluster, CLUSTER_IN_USE);

    return 0;
}

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

static openDir findDirEntry(const char *path)
{
    openDir location;
    dirEntry entry;
    char name[9];
    char ext[4];

    name[8] = 0;
    ext[3] = 0;
    extractNameAndExt(path, name, ext);

    //printf("name is: %s. extension is: %s.\n", name, ext);

    for (int i = 0; i < SECTOR; i+=32)
    {
        entry = readDirEntry(ROOT + i);
        //printf("first logical cluster %i, %i\n", i, entry.firstLogicalCluster);
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

int FAT_open(const char *path)
{
    openDir loc = findDirEntry(path);
    if(loc.dirLoc == -1)
        return -1;
    for(int i = 0; i < FILE_OPEN_LIMIT; i++)
    {
        if(openDirs[i].dirLoc == 0)
        {
            openDirs[i] = loc;
            return i;
        }
    }
    return -1;
}

/*
int FAT_write(int fdes, const char *buf, size_t size, size_t offset)
{
    openDir dir = openDirs[fdes];
    dirEntry entry = readDirEntry(dir.entryLoc);
    if (entry.firstLogicalCluster == 0)
    {
        int findNextFreeCluster()
    }
    ff_lseek(fd, fdes, SEEK_SET);
    ff_write(fd, buf, size);
}
*/

int FAT_close(int fd)
{
    memset(openDirs + fd, 0, sizeof(openDir));
    return 0;
}

#ifdef __cplusplus
}
#endif