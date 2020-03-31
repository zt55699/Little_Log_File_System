/* test3.c
 § Assumption: test2 was performed before this test. Vdisk properly
    initialized, and few files and folders added.
 § Test opening an existing valid vdisk with data inside.
 § Test writing the file in root directory
 § Test writing the file in sub-directory
 § Test update size of files and folders
 
 § Notice: Cannot be used to test a un-formatted disk, the vdisk to be open should be
    initialized correctly before.
 */
#include "../io/File.h"
#include "../disk/disk.h"
#include <unistd.h>

int main(){
    FILE* disk = fopen(vdisk_path, "rb+");
    Loading(disk);  //loading disk inodes/map and block_list into memory
    Print_structure(); //Print the loaded file structure
    //Write to a file
    
    printf("\n* Writing content under a block size into /root/readme.txt\n");
    char* pathname = "/root/readme.txt";
    // writing content under a block size
    unsigned char* content = (unsigned char*)"Hello World.";
    Writing(disk, pathname, content);
    Print_structure();
    printf("\n* Writing another content under a block size into /root/readme.txt\n\n");
    // writng another content under a block size
    content = (unsigned char*)"This is my little log file system.";
    Writing(disk, pathname, content);
    Print_structure();
    
    printf("\n* Writing content of multiple-block size (test file here is 3352 bytes) into sub-directory /root/folder1/LLFS.txt\n----------------------------\n");
    // writng a content of multiple blocks size, the file should less than 10 block size, which is less than 5120 bytes, the testfile "Text.txt" is under /apps.
    pathname = "/root/folder1/LLFS.txt";
    char**text= (char**)malloc(BLOCK_SIZE*10);
    if(read_extdata(text,"Text.txt") ==0){
        printf("ERROR: File does Not exist!\n");
        return -1;
    }
    //printf("Content:\n%s\n", *text);
    content = (unsigned char*) *text;
    Writing(disk, pathname, content);
    Print_structure();
    Print_map();
    //Print_blocks();
    fclose(disk);
    free(content);  //possibile bug here
    return 0;
}


