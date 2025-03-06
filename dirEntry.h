#pragma once
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define DIR_ENTRY_SIZE 32
#define SECTOR 512
#define FAT1 1 * SECTOR
#define FAT2 10 * SECTOR
#define ROOT 19 * SECTOR
#define DATA 33 * SECTOR

typedef struct
{
    char filename[9];
    char extension[4];
    uint32_t fileSize;
    uint16_t firstLogicalCluster;
    uint8_t attributes;
} dirEntry;

typedef struct
{
    int dirLoc;
    int entryLoc;
    int filePosition;
} openDir;

void writeDirEntry(int fd, dirEntry *entry, int offset);
dirEntry readDirEntry(int fd, size_t offset);
int findFreeDirEntry(int fd, int offset);
openDir findDirEntry(int fd, const char *path);