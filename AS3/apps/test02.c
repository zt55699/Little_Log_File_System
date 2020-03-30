/* test2.c
 § Test Initiallization of vdisk: creation and mounting
 § Test loading disk information to memory
 § Test creating root directory
 § Test creating flat file under root directory
 § Test creating sub-directories
 § Test updating inodes-map, block list
 */
#include "../io/File.h"
#include "../disk/disk.h"
#include <unistd.h>

int main(){
    InitLLFS();
    Print_structure(); 
    
    FILE* disk = fopen(vdisk_path, "rb+");
    //create flat files and create directories

    Createf(disk,"/root", "readme.txt", 0);
    Createf(disk,"/root", "folder1", 1);
    Createf(disk,"/root/folder1", "folder2", 1);
    Createf(disk,"/root", "folder3", 1);
    Createf(disk,"/root/folder1", "data3", 0);
    sleep(1);

    Print_map();
    Print_blocks();
    fclose(disk);
    return 0;
}



/*
//create sub_directory folder1 under root
path = "/root";
name = "folder1";
type = 1;
Createf(path, name, type);
Print_structure();
sleep(1);

//create sub_directory folder2 under folder1
path = "/root/folder1";
name = "folder2";
type = 1;
Createf(path, name, type);
Print_structure();
sleep(1);
//create sub_directory folder3 under root
path = "/root";
name = "folder3";
type = 1;
Createf(path, name, type);
Print_structure();
sleep(1);
//create sub_directory folder3 under root
path = "/root/folder1";
name = "data3";
type = 0;
Createf(path, name, type);
Print_structure();
sleep(1);
*/
