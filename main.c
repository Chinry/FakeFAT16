#include "fakeFat.h"
#include <stdio.h>

int main(int argc, char *argv[])
{  
    FAT_init();
    FAT_mknod("FILE.TXT");
    FAT_mknod("FILE2.TXT");
    FAT_exit();
    return 0;
}
