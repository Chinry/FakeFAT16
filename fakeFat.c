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
static int fd;

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
    memcpy(&entry.attributes, buffer + 11, sizeof(uint8_t));
    memcpy(&entry.firstLogicalCluster, buffer + 26, sizeof(uint16_t));
    memcpy(&entry.fileSize, buffer + 28, sizeof(uint32_t));
    return entry;
}

void FAT_init()
{
    uint8_t buf[DISK_SIZE];
    fd = open("storage.dat", O_RDWR|O_CREAT, 777);
    ftruncate(fd, DISK_SIZE); // 32kb
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
    entry.firstLogicalCluster = findNextFreeCluster();
    entry.fileSize = 0;
    writeDirEntry(&entry, dirOffset);
    setCluster(entry.firstLogicalCluster, CLUSTER_IN_USE);
    return 0;
}