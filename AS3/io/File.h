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

void InitLLFS(void); //Formatting the disk - initial set up and layout
struct filenode* newFile(char* path, char* name, int type); // Creating a new file, return a filenode struct for files[]
int Check_path(char* path); //return -1 if path is valid, return the aim file's inode# if valid.
int Find_folder_index(char* name); // given the file's filename, return its index in files[]
int Assign_afreeblock(void); // return a free block#, return -1 if full
struct filenode* LoadFiles(int type, int entries[] , struct String* name, int size); // Load directories info from vdisk
void Loading(FILE* disk); // Loading free block list, inode map, inodes, etc. into memory.
void Load_Structure(FILE* disk); // Loading file structure.
void Print_map(void); // Print out all used inodes
void Print_blocks(void); // Print out all used blocks
void Print_structure(void); // print out file structure by Loading()
void Print_files(void); // print out all files
void Update_blocklist(FILE* disk); // update the free blocklist to the vdisk
void Update_inodeMap(FILE* disk); // update the inodeMap to the vdisk
void Update_inodes(FILE* disk); // part of updating inode-map, not necessarily update inodes
void Update_directories(FILE* disk); // update directories
void Update_to_disk(FILE* disk); // Call all update here, write together.
void Createf(FILE* disk, char* path, char* name, int type); //Creating a file - given some directory location in tree hiarachy, name and type
void Writing(FILE* disk, char* pathname, unsigned char* content); // Writing to a file - append to an existing file
char* Reading(FILE* disk, char* pathname); // reading a file, load content into a buffer, then return it.
void Deleting (FILE* disk, char* pathname); // deletion of files and directories
int read_extdata(char **result,char *fileName); //Used for writing large data set into vdisk, from external source

void fsck(FILE* disk);//Checking the integrity of the inodes and the blocks

void Set_fsck (FILE* disk, int fsck_status); // Set fsck_status to Vdisk
void i_check(FILE* disk); // inodes check
void d_check(FILE* disk); // directories check
void Set_simulator(int sim); // 0 indicates turn off crash_simulator; 1 for crash scenario1, 2 for scenario2.
#endif
