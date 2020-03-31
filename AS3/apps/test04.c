/* test4.c
 § Assumption: test2 and test3 was performed before this test. Vdisk properly
    initialized, and few files and folders added and written.
 § Test reading the file in root directory
 § Test reading the file in sub-directory
 
 § Notice: Cannot be used to test a un-formatted disk, the vdisk to be open should be
    initialized correctly before.
 */
#include "../io/File.h"
#include "../disk/disk.h"
#include <unistd.h>

int main(){
    FILE* disk = fopen(vdisk_path, "rb");
    
    printf("* Read files in the root direcroty\n");
    char* readfile ="/root/readme.txt";
    char* read = Reading(disk, readfile); //Reading() returns a char* of content
    printf("Content read:\n----------------------------\n");
    printf("%s\n\n\n", read);
    free(read);
    
    printf("* Read files in the sub-direcroty\n");
    readfile = "/root/folder1/LLFS.txt";
    read = Reading(disk, readfile); //Reading() returns a char* of content
    printf("Content read:\n----------------------------\n");
    printf("%s\n\n\n", read);
    free(read);
    
    fclose(disk);
    return 0;
}


