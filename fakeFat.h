#pragma once
#include <stddef.h>

void FAT_init();
int FAT_open(const char *path);
int FAT_close(int fd);
int FAT_mkdir(const char *path);
int FAT_rmdir(const char *path);
int FAT_mknod(const char *path);
int FAT_read(int fd, char *buf, size_t size, size_t offset);
int FAT_write(int fdes, const char *buf, size_t size, size_t offset);
void FAT_remove(const char *path);
void FAT_exit();