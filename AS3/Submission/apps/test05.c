/* test5.c
 § Assumption: test2 and test3 was performed before this test. Vdisk properly
    initialized, and few files and folders added and written.
 § Test deletion of files and directories
 
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
    //Print_map();
   
    printf("* Delete a file in root directory\n");
    char* delefile ="/root/readme.txt";
    Deleting(disk, delefile);
    Print_structure();
    
    printf("* Delete a file in subdirectory\n");
    delefile ="/root/folder1/LLFS.txt";
    Deleting(disk, delefile);
    Print_structure();
    
    printf("* Delete an empty directory\n");
    delefile ="/root/folder3";
    Deleting(disk, delefile);
    Print_structure();
    
    Print_map();
    Print_blocks();
    //Print_files();
    
    fclose(disk);
    return 0;
}


