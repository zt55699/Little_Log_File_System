/*
 § You will want to test the functionality of your file system using many different options.
 § It is possible you could combine the features with your shell so that you could use commands like cd to traverse the directory structure. rm to remove files (both types). Create new files and then modify them. Find their size etc.
 § Its up to you to show us how you have implemented the functionality.
 */
#include "../io/File.h"
#include "../disk/disk.h"
#include <unistd.h>

int main(){
    InitLLFS();
    Print_folders();
    
    //create folder1 under root
    char* path = "/root";
    char* name = "folder1";
    int type = 1;
    Createf(path, name, type);
    Print_folders();
    sleep(1);
    //create folder2 under folder1
    path = "/root/folder1";
    name = "folder2";
    type = 1;
    Createf(path, name, type);
    Print_folders();
    sleep(1);
    //create folder3 under root
    path = "/root";
    name = "folder3";
    type = 1;
    Createf(path, name, type);
    Print_folders();
    sleep(1);

    Print_map();

    return 0;
}
