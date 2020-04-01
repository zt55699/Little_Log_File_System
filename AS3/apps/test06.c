/* test6.c
 § Assumption: test2 and test3 and test5 was performed before this test. Vdisk properly
    initialized, and few files and folders added and written.
 § Test the Robust of LLFS, the approach of "fsck"
 § Handle two scenarios: crashes that occur after free
    blocks have been allocated to a file, and crashes that occur just after blocks have
    been removed from the freelist.
 
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
   
    //Scenario 1: crashes that occur after free blocks have been allocated to a file
    //For example, crash occurs during the Writing() function, after free blocks been allocated.
    Set_simulator(1);
    char* pathname = "/root/crash.txt";
    unsigned char* content = (unsigned char*)"Crash simulation 1";
    Writing(disk, pathname, content);   // crash occurs here

    
    
    
    //Scenario 2: crashes that occur just after blocks have been removed from the freelist
    //For example, crash occurs during the Writing() function, just after blockes been removed.
    Set_simulator(2);
    
    
    
    
    fclose(disk);
    return 0;
}


