/* test3.c
 § Test opening an existing valid vdisk with data inside.
 § Test writing/reading the file in root directory
 § Test writing/reading the file in sub-directory
 § Test deleting files
 § Test creating directories
 
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
    //create flat files and create directories

    //Print_map();
    fclose(disk);
    return 0;
}


