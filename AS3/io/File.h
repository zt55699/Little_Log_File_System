#ifndef FILE_H
#define FILE_H
#define vdisk_path "../disk/vdisk"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define NUM_INODES 128 //not determined

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct String{
    char s[50];
} String;

struct filenode* newFile(char* path, char* name, int type);
int Check_path(char* path); //return -1 if path is valid, return the innerest file's inode# if valid.
int Find_folder_index(char* name); // given the file's filename, return its index in files[]

//Formatting the disk - initial set up and layout
void InitLLFS(void);

int Assign_afreeblock(void); // return a free block#, return -1 if full
struct filenode* LoadFiles(int type, int entries[] , struct String* name, int size);
void Loading(FILE* disk);
void Load_Structure(FILE* disk);
void Print_map(void);
void Print_blocks(void);
void Print_structure(void);
void Print_files(void);
void Update_blocklist(FILE* disk);
void Update_inodeMap(FILE* disk);
void Update_inodes(FILE* disk);
void Update_directories(FILE* disk);
void Update_to_disk(FILE* disk);

//Creating a file - given some directory location in tree hiarachy, name and type
void Createf(FILE* disk, char* path, char* name, int type);
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



void Writing(FILE* disk, char* pathname, unsigned char* content);
char* Reading(FILE* disk, char* pathname);
void Deleting (FILE* disk, char* pathname);



//Reading from a file - load a file from disk into memory




//Finding an inode - reading - modifying - deleting etc.



//Finding an available block



//Checking the integrity of the inodes and the blocks




int read_extdata(char **result,char *fileName);



#endif
