#include "File.h"
#include "../disk/disk.c"

int Fsck_status = 0;    // use this value to track whetehr the disk crashed or not
typedef struct Inode {
    int num;    //initialize to -1
    int location;
    int size;   //size of all files in bytes
    int flag;   //0 indicates flat file, 1 indicates directory
    int block_num[10];
    char name[50];
} Inode;
Inode InodeMap [128];
int num_useInode = 0;
int status =0;                     // status 0 indicates read a disk without Initial, 1ndicates a new disk
int blocklist [NUM_BLOCKS] = {0}; // 0 indicates the block is free

//Directory structure
#define MAX_PER_FOLDER 16
#define MAX_FOLDER_NUM 100
struct filenode{
    int type;   //0 indicates flat file, 1 indicates directory
    int inode_index[MAX_PER_FOLDER];    //position 0 is its own inode
    char* names[MAX_PER_FOLDER];    //position 0 is its own name
};
struct filenode* files[MAX_FOLDER_NUM];
struct filenode* newFile(char* path, char* name, int type){
    struct filenode* file = (struct filenode*)malloc(sizeof(struct filenode));
    
    file->type = type;
    for(int i =0; i<MAX_PER_FOLDER; i++){
        file->inode_index[i]=0;
        file->names[i] = NULL;
    }
       
  
    file->names[0] = name;
    int cm = strcmp(name, "root");
    if(cm == 0 && type==1){
        //printf("  root folder built\n");
        return(file);
    }
    // get the index of next free inode;
    int nextfree_inode = -1;
    for(int i =0; i<NUM_INODES; i++){
        if(InodeMap[i].location==0){
            nextfree_inode = i;
            break;
        }
    }
    // If cannot find a free inode return NULL pointer;
    if(nextfree_inode == -1)
        return NULL;
    file->inode_index[0]=nextfree_inode;
    // get the index of next free block;
    int nextfree_block = -1;
    for(int i =0; i<NUM_BLOCKS; i++){
        if(blocklist[i]==0){
            nextfree_block = i;
            blocklist[nextfree_block] = 1;
            break;
        }
    }
    // If cannot find a free block return NULL pointer;
    if(nextfree_block == -1)
        return NULL;
    // assign a free inode to the new directory file
    InodeMap[nextfree_inode].num = nextfree_inode;
    InodeMap[nextfree_inode].location= nextfree_block * BLOCK_SIZE; //inode position
    
    // If a directory inode, assgin its first direct block to directory information;
    nextfree_block = -1;
    if(type ==1){
        for(int i =0; i<NUM_BLOCKS; i++){
            if(blocklist[i]==0){
                nextfree_block = i;
                blocklist[nextfree_block] = 1;
                break;
            }
        }
        InodeMap[nextfree_inode].block_num[0] = nextfree_block;
    }
    
    
    // build directory's inode
    InodeMap[nextfree_inode].flag = type;
    // change parents files' entry, traverse to find the path
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
    int temppos = 1;
    for(int i =1; i<MAX_PER_FOLDER; i++){
        if(files[modify_folder]->inode_index[i] == 0){
            //assign one of the parent folder's free entry to this folder
            files[modify_folder]->inode_index[i]= nextfree_inode;
            printf("  %s: entry[%d] = %s\n", files[modify_folder]->names[0],i, name);
            files[modify_folder]->names[i] = name;
            break;
        }
        temppos ++;
    }
    if(temppos > MAX_PER_FOLDER)
        printf("full folder!\n");
    
    
    if(type==1)
        printf("New Folder %s created.\n", name);
    else
        printf("New File %s created.\n", name);
    return(file);
}

int Find_folder_index(char* name){
    for(int i = 0; i < MAX_FOLDER_NUM; i++){
        if(strcmp(files[i]->names[0], name)==0){
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
    //printf("num of level: %d\n", num_level);
    for(int j = 0; j<MAX_FOLDER_NUM; j++){
        if(files[j] == NULL){
            continue;
        }
        if(files[j]->type==1 && strcmp(files[j]->names[0],tempfolder[0])==0){
            //printf("%s == %s\n ",files[j]->names[0], tempfolder[0]);
            if(num_level == 1){
                printf("  path '%s' valid\n", path);
                return 0;
            }
            for(int k =0; k<MAX_PER_FOLDER; k++){
                if(files[j]->names[k] == NULL){
                    continue;
                }
                
                if(strcmp(files[j]->names[k] , tempfolder[1])==0){
                    //printf("%s == %s\n ",files[j]->names[k], tempfolder[1]);
                    if(num_level == 2){
                        printf("  path '%s' valid\n", path);
                        return 0;
                    }
                    for(int l =0; l<MAX_PER_FOLDER; l++){
                        if(files[Find_folder_index(files[j]->names[k])]->names[l] == NULL)
                            continue;
                        if(files[Find_folder_index(files[j]->names[k])]->type == 1 &&strcmp(files[Find_folder_index(files[j]->names[k])]->names[l], tempfolder[2])==0){
                            if(num_level == 3){
                                printf("  path '%s' valid\n", path);
                                return 0;
                            }
                            for(int m =0; m<MAX_PER_FOLDER; m++){
                                if(files[Find_folder_index(tempfolder[2])]->names[m] == NULL)
                                    continue;
                                if(strcmp(files[Find_folder_index(tempfolder[2])]->names[m], tempfolder[3])==0){
                                    if(num_level == 4){
                                        printf("  path '%s' valid\n", path);
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
                //printf("2nd level %s!=%s\n", files[j]->names[k],tempfolder[1]);
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
    printf("Start create/initialize the vdisk.\n");
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
    int block_ptr[10] = {9,0,0,0,0,0,0,0,0,0};
    int indirect = 0;
    memset(buffer, 0, 512);
    memcpy(buffer, &size, 4);   //4 bytes size
    memcpy(buffer + 4, &flags, 4); //4 bytes flags
    for (int i =0; i<10; i++){
        memcpy(buffer + 8 + i*2, &block_ptr[i], 2); //2 bytes*10 block_num
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
    files[0] = newFile("/", "root", 1);  // Initialize root directory
    files[0]->inode_index[0]=0;
    
    status =1;
    printf("Done initialization.\n");
    fclose(disk);
}


void Print_map(){
    printf("Inodes-map:\n");
    int count = 0;
    for(int i=0; i <NUM_INODES; i++){
        if (InodeMap[i].location!=0){
            if(InodeMap[i].flag ==0)
                printf("inode[%d] Location:%d Size:%d Type: FLAT_FILE\n", i, InodeMap[i].location, InodeMap[i].size);
            else
                printf("inode[%d] Location:%d Size:%d Type: DIRECTORY\n", i, InodeMap[i].location, InodeMap[i].size);
            if(InodeMap[i].flag==0){
                printf("    single-indirect blocks: ");
                for(int j = 0; j<10; j++){
                    printf("%d ", InodeMap[i].block_num[j]);
                }
            }
            else{
                printf("    Directory at block: ");
                printf("%d ", InodeMap[i].block_num[0]);
            }
            printf("\n");
            count++;
        }
    }
    num_useInode = count;
    printf("Total inodes: %d\n\n", num_useInode);
}

void Print_blocks(){
    printf("Blocks occupied:\n");
    int count = 0;
    for(int i=0; i<NUM_BLOCKS; i++){
        if(blocklist[i]==1){
            printf("%d ",i);
            count++;
        }
    }
    printf("\nTotal used blocks: %d\n\n",count);
}

// print out file structure in memory, loading() first
void Print_structure(){
    printf("files structure:\n");
    printf("  /root\n");
    for(int i=1; i<MAX_PER_FOLDER; i++){
        if(files[0]->names[i] == NULL)
            continue;
        else{
            printf("    |--%s\n",  files[0]->names[i]);
            if(files[Find_folder_index(files[0]->names[i])]->type == 0){
                continue;
            }
            for(int j =1; j<MAX_PER_FOLDER; j++){
                if(files[Find_folder_index(files[0]->names[i])]->names[j]==NULL){
                    continue;
                }
                else{
                    printf("        |--%s\n",  files[Find_folder_index(files[0]->names[i])]->names[j]);
                    if(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->type==0)
                        continue;
                    for(int k =1; k<MAX_PER_FOLDER; k++){
                        if(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k]==NULL)
                            continue;
                        else{
                            printf("            |--%s\n",  files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k]);
                            if(files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->type==0)
                                    continue;
                            for(int l =1; l<MAX_PER_FOLDER; l++){
                            if(files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->names[l]==NULL)
                                    continue;
                                else{
                                    printf("                |--%s\n",  files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->names[j]);
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

void Print_files()
{
    for(int i = 0; i <MAX_FOLDER_NUM; i++){
        if(files[i]!=NULL){
            if(files[i]->type == 1)
                printf(" /%s: ", files[i]->names[0]);
            else
                printf("  %s", files[i]->names[0]);
            for(int j =1; j <MAX_PER_FOLDER; j++){
                if(files[i]->names[j]!=NULL)
                    printf(" [%d]%s", j, files[i]->names[j]);
            }
            printf("\n");
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
    
    
    //loading file structure
    if(status==0){
        printf("Loading Structure from disk\n");
        Load_Structure(disk);
    }
    //print inode map
    
    //printf("Loading Inodes&map and block_list from disk:\n");
    
    //Print_map();
    //Print_blocks();
    printf("----------------------------\n");
    printf("VDisk loaded.\n");
}

// Load file-structure into memory
void Load_Structure(FILE* disk){
    for(int x= 0; x<MAX_FOLDER_NUM; x++){
        if(files[x] != NULL)
            free(files[x]);
    }
    unsigned char * buffer = (unsigned char *) malloc(512);
    char* textbuff;
    int tail =0;
    for(int i=0; i <NUM_INODES; i++){
        if(InodeMap[i].flag ==1){
            int type;
            int entries[MAX_PER_FOLDER] = {0};
            String names[MAX_PER_FOLDER];
            type = InodeMap[i].flag;
            //printf("seek location:%d\n",InodeMap[i].block_num[0]*512 );
            fseek(disk, InodeMap[i].block_num[0]*512, SEEK_SET);
            //load the folder information
            fread(buffer, 512, 1, disk);
            int sizecount = 0;
            //printf("InodeMap[%d] is folder: \n", i);
            for(int j =0; j< 16; j++){
                textbuff = (char*)malloc(31);
                entries[j]= buffer[j*32];
                fseek(disk, InodeMap[i].block_num[0]*512+j*32+1, SEEK_SET);
                fread(textbuff, 31, 1, disk);
                
                //printf("textbuffer:\n");
                //for(int s = 0;s < 31; s++){
                //    printf("%.2X ", (int)textbuff[s]);
                //}
                //printf("\n");
            
                strcpy(names[j].s , textbuff);
                if(strlen(names[j].s) == 0){
                    free(textbuff);
                    break;
                }
                sizecount++;
                //printf("    entry:%d name:%s\n", entries[j], names[j].s);
                strcpy(InodeMap[buffer[j*32]].name , names[j].s);
                free(textbuff);
            }
            files[tail] = LoadFiles(type, entries , names, sizecount);
            tail ++;
                
            
        }
        
    }
    //printf("%d folders loaded!\n", tail);
    for(int i=0; i <NUM_INODES; i++){
        String names2[MAX_PER_FOLDER];
        if(InodeMap[i].location!=0 &&InodeMap[i].flag ==0){
            int entries[MAX_PER_FOLDER];
            char na[0] = "";
            for(int k = 1; k<MAX_PER_FOLDER; k++){
                entries[k] = -1;
                strcpy(names2[k].s,na);
            }
            entries[0] = InodeMap[i].num;
            strcpy(names2[0].s,InodeMap[i].name);
            files[tail] = LoadFiles(InodeMap[i].flag, entries , names2, 1);
            tail ++;
        }
    }
    
    free(buffer);
    printf("%d files loaded:\n", tail);
    Print_files();
    status = 1;
    
}


struct filenode* LoadFiles(int type, int entries[] ,struct String* name, int size){
    struct filenode* file = (struct filenode*)malloc(sizeof(struct filenode));
    file->type = type;
    for(int i =0; i <size; i++){
        file->inode_index[i] = entries[i];
        file->names[i] = (char*)malloc(100);
        strcpy(file->names[i],name[i].s);
        //file->names[i] = name[i].s;
        //printf("    Copy %s to filename, now it is %s\n",name[i].s, file->names[i] );
    }
    return(file);
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
    printf("  Blocklist updated.\n");
}

void Update_inodeMap(FILE* disk)
{
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    
    for(int i=0; i<NUM_INODES; i++){
        int num = InodeMap[i].num;
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
    printf("  Inodes_map updated.\n");
}

void Update_inodes(FILE* disk)
{
    unsigned char* buffer;
    for(int i=0; i<NUM_INODES; i++){
        buffer = (unsigned char *) malloc(512);
        int size = InodeMap[i].size;
        int flag = InodeMap[i].flag;
        if(InodeMap[i].location!=0){
            memset(buffer, 0, 512);
            memcpy(buffer, &size, 4);
            memcpy(buffer + 4, &flag, 4);
            for (int j =0; j<10; j++){
                memcpy(buffer + 8 + j*2, &InodeMap[i].block_num[j], 2);
            }
            writeBlock(disk, InodeMap[i].location/512, buffer);
            free(buffer);
        }
    }
    printf("  Inodes updated.\n");
}

void Update_directories(FILE* disk)
{
    unsigned char* buffer;
    for(int i=0; i<MAX_FOLDER_NUM; i++){
        if(files[i]!=NULL){
            if(files[i]->type==1){
                //printf("Update file[%d] index:%d, name:%s\n", i, files[i]->inode_index[0], files[i]->names[0]);
                buffer = (unsigned char *) malloc(512);
                memset(buffer, 0, 512);
                for(int j=0; j<MAX_PER_FOLDER; j++){
                    memcpy(buffer+j*32, &files[i]->inode_index[j], 1);
                    if(files[i]->names[j]!=NULL){
                        //printf("    :%s\n",files[i]->names[j]);
                        memcpy(buffer+j*32+1, files[i]->names[j], strlen(files[i]->names[j]));
                    }
                }
                writeBlock(disk, InodeMap[files[i]->inode_index[0]].block_num[0], buffer);
                //printf("---Write %s to postion %d\n", buffer, InodeMap[files[i]->inode_index[0]].block_num[0]*512);
                free(buffer);
            }
        }
    }
    
    //Print_files();
    printf("  Directories updated.\n");
}

void Update_to_disk(FILE* disk)
{
    printf("Write updates to disk\n");
    //update the free blocklist to the vdisk
    Update_blocklist(disk);
    Update_inodeMap(disk);
    Update_inodes(disk);
    Update_directories(disk);
    printf("All updates written to disk, fsck_status=FALSE\n----------------------------\n\n");
}

//Creating a file - given some directory location in tree hiarachy, name and type
/*
§ Check to see if there are any free inode blocks
§ If there are then create new inode with name, type of file (directory or regular) etc.
§ Check if there are any free data blocks and if so then allocate one for the new file.
§ Create a new directory entry for the new file in the given directory location
§ If any of this can’t be done then return an error
 */
void Createf(FILE* disk, char* path, char* name, int type)
{
    
    Loading(disk);
    if (type ==0)
        printf("Try create flat file: %s\n", name);
    else if (type ==1)
        printf("Try create directory: %s\n", name);
    for(int i =1; i<MAX_FOLDER_NUM; i++){
            if(files[i]==NULL){
                files[i] = newFile(path, name, type);
                if(files[i]==NULL){
                    printf("Error creating file!\n");
                    return;
                }
                break;
            }
    }
    //printf("print inodes from memory:\n");
    //Print_map();
    //printf("done creating\n\n");
    Print_structure();
    Update_to_disk(disk);
    
}

//Writing to a file - append to an existing file
void Writing(FILE* disk, char* filename, char* buffer)
{
    Loading(disk);
    
    
    
    
    Update_to_disk(disk);
    printf("done writing\n\n");
}

