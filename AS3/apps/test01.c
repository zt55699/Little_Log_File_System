/*
 § You will want to test the functionality of your file system using many different options.
 § It is possible you could combine the features with your shell so that you could use commands like cd to traverse the directory structure. rm to remove files (both types). Create new files and then modify them. Find their size etc.
 § Its up to you to show us how you have implemented the functionality.
 */
#include "../io/File.h"
#include "../disk/disk.h"

int main(){
    //intialize the vdisk file
    InitLLFS();
    
    //test read block to the memory
    FILE* disk = fopen(vdisk_path, "rb+");
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    readBlock(disk, 1, buffer); //read free block list into memory
    printf("Read Block1 in HEX: \n");
    print_hexbuffer(buffer);
    free(buffer);
    
    //test write block to the memory
    printf("Write 1s to Block1\n");
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    memset(buffer, 1, BLOCK_SIZE);
    writeBlock(disk, 1, buffer);
    free(buffer);
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    readBlock(disk, 1, buffer); //read free block list into memory
    printf("Read Block1 in HEX: \n");
    print_hexbuffer(buffer);
    free(buffer);
    fclose(disk);
    

    return 0;
}
