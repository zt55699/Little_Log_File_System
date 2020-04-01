/* test7_crash_sim2.c
 This is a scenario 2 crash simulator.
 crashes that occur just after blocks have been removed from the freelist
 For example, crash occurs during the Writing() function, just after blockes been removed.
 
 § Assumption: test2,3,4,5 was sequentially performed before this test. Vdisk properly
    initialized, and few files and folders added and written.

 § Notice: Cannot be used to test a un-formatted disk, the vdisk to be open should be
    initialized correctly before.
 */
#include "../io/File.h"
#include "../disk/disk.h"
#include <unistd.h>

int main(){
    FILE* disk = fopen(vdisk_path, "rb+");
    Loading(disk);  //loading disk inodes/map and block_list into memory
    //Print_structure(); //Print the loaded file structure
    //Print_map();
    //Print_blocks();
   
    //Scenario 2: crashes that occur just after blocks have been removed from the freelist
    //For example, crash occurs during the Writing() function, just after blockes been removed.
    Set_simulator(2); // 0 indicates turn off crash_simulator; 1 for crash scenario1, 2 for scenario2.
    char* pathname = "/root/crash.txt";
    unsigned char* content = (unsigned char*)"Crash simulation 2";
    Writing(disk, pathname, content);   // crash occurs here
    
    
    fclose(disk);
    return 0;
}


