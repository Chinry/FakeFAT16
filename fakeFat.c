#ifdef __cplusplus
extern "C" {
#endif

#include "fakeFat.h"
#include "ff_actions.h"
#include "dirEntry.h"





#define DISK_SIZE 64 * 1024

#define CLUSTER_END 0xFFFF
#define ATTR_DIRECTORY 0x10

#define FILE_OPEN_LIMIT 64

static int fd;

static openDir openDirs[FILE_OPEN_LIMIT];



static int findNextFreeCluster()
{
    uint16_t buffer;
    ff_lseek(fd, FAT1 + 2 * sizeof(uint16_t), SEEK_SET);
    do{
        ff_read(fd, &buffer, sizeof(uint16_t));
    }while(buffer != 0);
    return (ff_lseek(fd, 0, SEEK_CUR) - (FAT1 + sizeof(uint16_t))) / 2;
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



static void setCluster(uint16_t clusterNum, uint16_t set)
{
    ff_lseek(fd, FAT1 + (clusterNum * sizeof(uint16_t)), SEEK_SET);
    ff_write(fd, &set, sizeof(uint16_t));
}

int FAT_mknod(const char *path)
{
    int dirOffset = findFreeDirEntry(fd, ROOT);
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
    writeDirEntry(fd, &entry, dirOffset);
    return 0;
}

int FAT_mkdir(const char *path)
{
    int dirOffset = findFreeDirEntry(fd, ROOT);
    dirEntry entry;
    entry.attributes = ATTR_DIRECTORY;
    memset(entry.filename, ' ', 8);
    memcpy(entry.filename, path, strlen(path));
    memset(entry.extension, ' ', 3);
    entry.firstLogicalCluster = findNextFreeCluster();
    writeDirEntry(fd, &entry, dirOffset);
    setCluster(entry.firstLogicalCluster, CLUSTER_END);

    return 0;
}





int FAT_open(const char *path)
{
    openDir loc = findDirEntry(fd, path);
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