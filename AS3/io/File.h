#ifndef FILE_H
#define FILE_H
#define vdisk_path "../disk/vdisk"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define NUM_INODES 128 //not determined

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct directory* newDirectory(char* path, char* name);
int Check_path(char* path);
int Find_folder_index(char* name);
//Formatting the disk - initial set up and layout
void InitLLFS(void);



void Loading(FILE* disk);
void Print_map(void);
void Print_folders(void);
void Update_blocklist(FILE* disk);
void Update_inodeMap(FILE* disk);
void Update_to_disk(void);

//Creating a file - given some directory location in tree hiarachy, name and type
void Createf(char* path, char* name, int type);
/*
§ Check to see if there are any free inode blocks
§ If there are then create new inode with name, type of file (directory or regular) etc.
§ Check if there are any free data blocks and if so then allocate one for the new file.
§ Create a new directory entry for the new file in the given directory location
§ If any of this can’t be done then return an error
 */






//Deleting a file - from a given directory
/*
 § Find the inode for that file given its directory listing and name
 § Get the location of the files blocks from the inode
 § Check if the file is regular file or directory – if it is directory check if it is empty or not
 § Deallocate the blocks listed in the inode – set them as free in block vector
 § Remove file listing from directory
 § Deallocate inode and set its location as free
 */





//Writing to a file - append to an existing file




//Reading from a file - load a file from disk into memory




//Finding an inode - reading - modifying - deleting etc.



//Finding an available block



//Checking the integrity of the inodes and the blocks







#endif
