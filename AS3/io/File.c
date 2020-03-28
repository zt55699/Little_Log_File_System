#include "File.h"
#include "../disk/disk.c"

typedef struct Inode {
    int num;    //initialize to -1
    int location;
    int size;   //size of all files in bytes
    int flag;   //0 indicates flat file, 1 indicates directory
    int block_num[10];
} Inode;
Inode InodeMap [128];
int num_useInode = 0;
int blocklist [4096] = {0}; // 0 indicates the block is free

//Directory structure
#define MAX_PER_FOLDER 10
#define MAX_FOLDER_NUM 100
struct directory{
    int inode_index[MAX_PER_FOLDER];    //position 0 is its own inode
    char* names[MAX_PER_FOLDER];
};
struct directory* folders[MAX_FOLDER_NUM];
struct directory* newDirectory(char* path, char* name){
    struct directory* folder = (struct directory*)malloc(sizeof(struct directory));
    for(int i =0; i<MAX_PER_FOLDER; i++){
        folder->inode_index[i]=0;
        folder->names[i] = NULL;
    }
    folder->names[0] = name;
    int cm = strcmp(name, "root");
    if(cm == 0){
        printf("build root folder\n");
        return(folder);
    }
    // get the index of next free inode;
    int nextfree_inode = 0;
    for(int i =0; i<NUM_INODES; i++){
        if(InodeMap[i].location==0){
            nextfree_inode = i;
            break;
        }
    }
    folder->inode_index[0]=nextfree_inode;
    // get the index of next free block;
    int nextfree_block = 0;
    for(int i =0; i<NUM_BLOCKS; i++){
        if(blocklist[i]==0){
            nextfree_block = i;
            break;
        }
    }
    // assign a free inode to the new directory file
    InodeMap[nextfree_inode].num = nextfree_inode;
    InodeMap[nextfree_inode].location= nextfree_block * BLOCK_SIZE; //inode position
    // build directory's inode
    InodeMap[nextfree_inode].flag = 1;
    // change parents folders' entry, traverse to find the path
    int num_level = 0;
    char tempath[50] = {0};
    strncpy(tempath, path, strlen(path));
    char* tempfolder[5];
    const char* delim = "/";
    char* token;
    token = strtok(tempath, delim);
    while (token!=NULL){
        tempfolder[num_level] = token;
        //printf("tempfolder[%d]:'%s'\n",num_level, tempfolder[num_level]);
        num_level++;
        token = strtok(NULL, delim);
    }
    //printf("num of level: %d\n", num_level);
    // check path validation
    if(Check_path(path)==-1){
        printf("Failed to create folder %s\n", name);
        return NULL;
    }
    //printf("start modify upper folder %s's inode\n", tempfolder[num_level-1]);
    //change most inside level folder's entry
    
    int modify_folder = Find_folder_index(tempfolder[num_level-1]);
    for(int i =1; i<MAX_PER_FOLDER; i++){
        if(folders[modify_folder]->inode_index[i] == 0){
            //assign one of the parent folder's free entry to this folder
            folders[modify_folder]->inode_index[i]= nextfree_inode;
            folders[modify_folder]->names[i] = name;
            break;
        }
        else
            printf("full folder!\n");
        
    }
    
    blocklist[nextfree_block] = 1;
    
    printf("New Folder %s created.\n", name);
    return(folder);
}

int Find_folder_index(char* name){
    for(int i = 0; i < MAX_FOLDER_NUM; i++){
        if(strcmp(folders[i]->names[0], name)==0){
            return i;
        }
    }
    return -1;
}

// return 0 if path is valid, return -1 if path is invalid
int Check_path(char* path){
    int num_level = 0;
    char tempath[50] = {0};
    strncpy(tempath, path, strlen(path));
    char* tempfolder[5];
    const char* delim = "/";
    char* token;
    token = strtok(tempath, delim);
    while (token!=NULL){
        tempfolder[num_level] = token;
        num_level++;
        token = strtok(NULL, delim);
    }
    
    for(int j = 0; j<MAX_FOLDER_NUM; j++){
        if(folders[j] == NULL){
            continue;
        }
        if(strcmp(folders[j]->names[0],tempfolder[0])==0){
            //printf("%s == %s\n ",folders[j]->names[0], tempfolder[0]);
            if(num_level == 1){
                printf("path '%s' valid\n", path);
                return 0;
            }
            for(int k =0; k<MAX_PER_FOLDER; k++){
                if(folders[j]->names[k] == NULL){
                    continue;
                }
                if(strcmp(folders[j]->names[k] , tempfolder[1])==0){
                    //printf("%s == %s\n ",folders[j]->names[k], tempfolder[1]);
                    if(num_level == 2){
                        printf("path '%s' valid\n", path);
                        return 0;
                    }
                    for(int l =0; l<MAX_PER_FOLDER; l++){
                        if(folders[Find_folder_index(folders[j]->names[k])]->names[l] == NULL)
                            continue;
                        if(strcmp(folders[Find_folder_index(folders[j]->names[k])]->names[l], tempfolder[2])==0){
                            if(num_level == 3){
                                printf("path '%s' valid\n", path);
                                return 0;
                            }
                            for(int m =0; m<MAX_PER_FOLDER; m++){
                                if(folders[Find_folder_index(tempfolder[2])]->names[m] == NULL)
                                    continue;
                                if(strcmp(folders[Find_folder_index(tempfolder[2])]->names[m], tempfolder[3])==0){
                                    if(num_level == 4){
                                        printf("path '%s' valid\n", path);
                                        return 0;
                                    }
                                    if(num_level >4){
                                        printf("Only support at most 4 levels of directories\n");
                                        return -1;
                                    }
                                }
                            }
                            printf("4th level path not exist!\n");
                            return -1;
                        }
                    }
                    printf("3rd level path not exist!\n");
                    return -1;
                }
                //printf("2nd level %s!=%s\n", folders[j]->names[k],tempfolder[1]);
            }
            printf("2nd level path not exist!\n");
            return -1;
        }
    }
    printf("1st level path not exist!\n");
    return -1;
}


//Formatting the disk - initial set up and layout
void InitLLFS()
{
    printf("Start initialize the disk file.\n");
    FILE* disk = fopen(vdisk_path, "wb+");
    char* init = calloc(BLOCK_SIZE * NUM_BLOCKS, 1); //init 0s
    fwrite(init, BLOCK_SIZE * NUM_BLOCKS, 1, disk);
    free(init);
    fclose(disk);
    
    disk = fopen(vdisk_path, "rb+");
    
    // Initialize Block 0
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    int magic = 42;
    int blocks = NUM_BLOCKS;
    int inodes = NUM_INODES;
    memcpy(buffer, &magic, sizeof(magic));
    memcpy(buffer + sizeof(int)*1, &blocks, sizeof(int));
    memcpy(buffer + sizeof(int)*2, &inodes, sizeof(int));
    writeBlock(disk, 0, buffer);
    free(buffer);
    
    // Initialize Block 1 - free block vector, first 10 not available for data
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    unsigned char freeBlock_buffer[BLOCK_SIZE];
    init_FBbuffer(freeBlock_buffer, BLOCK_SIZE);
    memcpy(buffer, &freeBlock_buffer[0], sizeof(freeBlock_buffer));
    //buffer = &freeBlock_buffer[0];
    writeBlock(disk, 1, buffer);
    free(buffer);
    
    // Initialize Block 2 - Inode map
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    int inode_num = 0;      //inode number  1byte
    int location = 1536;    //location of the inode   3byte
    memset(buffer, 0, 512);
    memcpy(buffer, &inode_num, 1);
    memcpy(buffer + 1, &location, 3);
    writeBlock(disk, 2, buffer);
    free(buffer);
    
    // Initialize Block 3 - initial inode for root directory
    // An inode has to be allocated to represent information for the root directory.
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    int size = 512;
    int flags = 1;
    int block_ptr[10] = {10,0,0,0,0,0,0,0,0,0};
    int indirect = 0;
    memset(buffer, 0, 512);
    memcpy(buffer, &size, 4);
    memcpy(buffer + 4, &flags, 4);
    for (int i =0; i<10; i++){
        memcpy(buffer + 8 + i*2, &block_ptr[i], 2);
    }
    memcpy(buffer + 8+2*10, &indirect, 4);
    writeBlock(disk, 3, buffer);
    free(buffer);

    // Initialize Block 9 - root directory.
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    unsigned int entries[16] = {0};
    entries[0] = 0; // inode 0 means no entry, reserved for root directory
    char filename[31] = "root\0";   // Filename
    memcpy(buffer, &entries[0], 1); // Inode index of Subdirectories or files
    memcpy(buffer + 1, filename, sizeof(filename));
    memset(buffer + 32, 0, 32*15);
    writeBlock(disk, 9, buffer);
    free(buffer);
    folders[0] = newDirectory("/", "root");  // Initialize root directory
    folders[0]->inode_index[0]=0;
    
    Loading(disk);
    printf("Done initialization.\n");
    fclose(disk);
}


void Print_map(){
    printf("Inodes-map:\n");
    int count = 0;
    for(int i=0; i <NUM_INODES; i++){
        if (InodeMap[i].location!=0){
            printf("inode[%d] location:%d size:%d flag:%d\n", i, InodeMap[i].location, InodeMap[i].size, InodeMap[i].flag);
            printf("    direct block number: ");
            for(int j = 0; j<10; j++){
                printf("%d ", InodeMap[i].block_num[j]);
            }
            printf("\n");
            count++;
        }
    }
    num_useInode = count;
    printf("Total inodes: %d\n\n", num_useInode);
}

void Print_folders(){
    printf("folders structure:\n");
    printf("  /root\n");
    for(int i=1; i<MAX_PER_FOLDER; i++){
        if(folders[0]->names[i] == NULL)
            continue;
        else{
            printf("    |--%s\n",  folders[0]->names[i]);
            for(int j =1; j<MAX_PER_FOLDER; j++){
                if(folders[Find_folder_index(folders[0]->names[i])]->names[j]==NULL)
                    continue;
                else{
                    printf("        |--%s\n",  folders[Find_folder_index(folders[0]->names[i])]->names[j]);
                    for(int k =1; k<MAX_PER_FOLDER; k++){
                        if(folders[Find_folder_index(folders[Find_folder_index(folders[0]->names[i])]->names[j])]->names[k]==NULL)
                            continue;
                        else{
                            printf("            |--%s\n",  folders[Find_folder_index(folders[Find_folder_index(folders[0]->names[i])]->names[j])]->names[k]);
                            for(int l =1; l<MAX_PER_FOLDER; l++){
                            if(folders[Find_folder_index(folders[Find_folder_index(folders[Find_folder_index(folders[0]->names[i])]->names[j])]->names[k])]->names[l]==NULL)
                                    continue;
                                else{
                                    printf("                |--%s\n",  folders[Find_folder_index(folders[Find_folder_index(folders[Find_folder_index(folders[0]->names[i])]->names[j])]->names[k])]->names[j]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    printf("\n");
}



void Loading(FILE* disk){
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    //read free block list into memory
    readBlock(disk, 1, buffer);
    for(int i =0; i <BLOCK_SIZE; i++){
        for(int j =0; j <8; j++){
            unsigned char* temp = (unsigned char *) malloc(1);
            memcpy(temp, &buffer[i], 1);
            *temp = *temp>>j;
            *temp &= 1UL;
            blocklist[i*8+j] = *temp;
            //printf("%d:%d ", i*8+j, blocklist[i*8+j]);
            free(temp);
        }
    }
    free(buffer);
    
    //read inode map into memory
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    readBlock(disk, 2, buffer);
    for(int i =0; i <BLOCK_SIZE/4; i++){
        InodeMap[i].num = buffer[i*4];
        int temp1 = buffer[i*4+1]*16*16*16*16;
        int temp2 = buffer[i*4+2]*16*16;
        int temp3 = buffer[i*4+3];
        int location = temp1+temp2+temp3;
        InodeMap[i].location = location;
        if(InodeMap[i].location!=0)
            num_useInode++;
    }
    free(buffer);
    
    //read inode into memory
    buffer = (unsigned char *) malloc(32);
    for(int i=0; i <NUM_INODES; i++){
        if (InodeMap[i].location!=0){
            fseek(disk, InodeMap[i].location, SEEK_SET);
            //load the 32 bytes inode
            fread(buffer, 32, 1, disk);
            //load first 4 bytes file size
            int temp1 = buffer[0];
            int temp2 = buffer[1]*16*16;
            int temp3 = buffer[2];
            int temp4 = buffer[3];
            int filesize = temp1+temp2+temp3+temp4;
            InodeMap[i].size = filesize;
            //load next 4 bytes flag
            int tem1 = buffer[4];
            int tem2 = buffer[5];
            int tem3 = buffer[6];
            int tem4 = buffer[7];
            int flag = tem1+tem2+tem3+tem4;
            InodeMap[i].flag = flag;
            //load next 2bytes* 10 block num
            for(int j= 0; j<10; j++){
                int te1 = buffer[8+j*2+1]*16*16;
                int te2 = buffer[8+j*2];
                int blocknum = te1+te2;
                InodeMap[i].block_num[j] = blocknum;
            }
        }
    }
    free(buffer);
    
    
    //print inode map
    printf("Loading Inode-map from disk:\n");
    Print_map();
    

    printf("done loading\n");
}

//update the free blocklist to the vdisk
void Update_blocklist(FILE* disk)
{
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    unsigned char freeBlock_buffer[BLOCK_SIZE]={0};
    
    for(int i=0; i<NUM_BLOCKS; i++){
        if(blocklist[i]==1) // 1 indicates the block is used
            set_block(freeBlock_buffer, i);
    }
    memcpy(buffer, &freeBlock_buffer[0], sizeof(freeBlock_buffer));
    writeBlock(disk, 1, buffer);
    free(buffer);
    printf("Blocklist updated.\n");
}

void Update_inodeMap(FILE* disk)
{
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    
    for(int i=0; i<NUM_INODES; i++){
        int num = i;
        int location = InodeMap[i].location;
        if(location!=0){
            memcpy(buffer+i*4, &num, 1);
        }
        else{
            num = 0;
            memcpy(buffer+i*4, &num, 1);
        }
        memcpy(buffer+i*4+1, &location, 3);
    }
    writeBlock(disk, 2, buffer);
    free(buffer);
    printf("Inodes_map updated.\n");
}


//Creating a file - given some directory location in tree hiarachy, name and type
/*
§ Check to see if there are any free inode blocks
§ If there are then create new inode with name, type of file (directory or regular) etc.
§ Check if there are any free data blocks and if so then allocate one for the new file.
§ Create a new directory entry for the new file in the given directory location
§ If any of this can’t be done then return an error
 */
void Createf(char* path, char* name, int type)
{
    
    
    if (type ==1){
        printf("start to create directory: %s\n", name);
        for(int i =1; i<MAX_FOLDER_NUM; i++){
            if(folders[i]==NULL){
                folders[i] = newDirectory(path, name);
                if(folders[i]==NULL){
                    return;
                }
                break;
            }
        }
    }
    Update_to_disk();
    printf("done creating\n\n");
}



void Update_to_disk()
{
    FILE* disk = fopen(vdisk_path, "rb+");
    //update the free blocklist to the vdisk
    Update_blocklist(disk);
    Update_inodeMap(disk);
    fclose(disk);
    printf("Updated to disk\n");
}
