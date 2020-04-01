/* test6_fsckSim.c
 this tests the fsck function for a crushed vdisk.
 § Assumption: test2,3,4,5and 6CrashSim was sequentially performed before this test.
 § Test the Robust of LLFS, the approach of "fsck"
 § Handle two scenarios: crashes that occur after free
    blocks have been allocated to a file, and crashes that occur just after blocks have
    been removed from the freelist. This is the crashed scenario 1;
 
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
    Print_map();
    Print_blocks();
   
    
    fclose(disk);
    return 0;
}


