/* test01.c
 § tests simple functionality: read/write to vdisk; initialize vdisk.
 § Notice: after this test, the vdisk was modified, no longer valid format, needs do
    InitLLFS() for later operation.
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
    readBlock(disk, 11, buffer); //read free block list into memory
    printf("Read Block11 in HEX: \n");
    print_hexbuffer(buffer);
    free(buffer);
    
    //test write block to the memory
    printf("Write 1s to Block11\n");
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    memset(buffer, 1, BLOCK_SIZE);
    writeBlock(disk, 11, buffer);
    free(buffer);
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    readBlock(disk, 11, buffer); //read free block list into memory
    printf("Read Block11 in HEX: \n");
    print_hexbuffer(buffer);
    free(buffer);
    fclose(disk);
   

    return 0;
}
