#include "File.h"
#include "../disk/disk.c"
int Crash_simulator = 0; // 0 indicates turn off crash_simulator; 1 for crash scenario1, 2 for scenario2.
int Fsck_status = 0;    // use this value to track whetehr the disk crashed or not. 0 means no need fsck
typedef struct Inode {
    int num;    //initialize to -1
    int location;   //absolute location Not block#
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


// given the file's filename, return its index in files[]
int Find_folder_index(char* name){
    if(strlen(name)<=0){
        printf (    "Try find empty string exit!/n");
        exit (0);
    }
    for(int i = 0; i < MAX_FOLDER_NUM; i++){
        if(files[i]!=NULL){
            if(strcmp(files[i]->names[0], name)==0){
                return i;
            }
        }
    }
    printf("ERROR: Cannot find %s in files[]\n",name);
    return -1;
}

//return -1 if path is valid, return the innerest file's inode# if valid.
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
                printf("  aim path '%s' valid\n", path);
                return files[j]->inode_index[0];
            }
            for(int k =0; k<MAX_PER_FOLDER; k++){
                if(files[j]->names[k] == NULL){
                    continue;
                }
                
                if(strcmp(files[j]->names[k] , tempfolder[1])==0){
                    //printf("%s == %s\n ",files[j]->names[k], tempfolder[1]);
                    if(num_level == 2){
                        printf("  aim path '%s' valid\n", path);
                        return files[j]->inode_index[k];
                    }
                    for(int l =0; l<MAX_PER_FOLDER; l++){
                        if(files[Find_folder_index(files[j]->names[k])]->names[l] == NULL)
                            continue;
                        if(files[Find_folder_index(files[j]->names[k])]->type == 1 &&strcmp(files[Find_folder_index(files[j]->names[k])]->names[l], tempfolder[2])==0){
                            if(num_level == 3){
                                printf("  aim path '%s' valid\n", path);
                                return files[Find_folder_index(files[j]->names[k])]->inode_index[l];
                            }
                            for(int m =0; m<MAX_PER_FOLDER; m++){
                                if(files[Find_folder_index(tempfolder[2])]->names[m] == NULL)
                                    continue;
                                if(strcmp(files[Find_folder_index(tempfolder[2])]->names[m], tempfolder[3])==0){
                                    if(num_level == 4){
                                        printf("  aim path '%s' valid\n", path);
                                        return files[Find_folder_index(tempfolder[2])]->inode_index[m];
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
    int size = 0;
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
    
    // Initialize Block 8 - initial fsck_status indicator
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    int fsck_indicator = 0;
    memset(buffer, 0, 512);
    memcpy(buffer, &fsck_indicator, sizeof(int));
    writeBlock(disk, 8, buffer);
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

//Print out all used inodes
void Print_map(){
    printf("Inodes-map:\n");
    int count = 0;
    for(int i=0; i <NUM_INODES; i++){
        if (InodeMap[i].location!=0){
            if(InodeMap[i].flag ==0)
                printf("inode[%d] Block:%d Size:%d Type: FLAT_FILE\n", i, InodeMap[i].location/512, InodeMap[i].size);
            else
                printf("inode[%d] Block:%d Size:%d Type: DIRECTORY\n", i, InodeMap[i].location/512, InodeMap[i].size);
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

//Print out all used blocks
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
    printf("  /root             [Total size:%d]\n", InodeMap[0].size);
    for(int i=1; i<MAX_PER_FOLDER; i++){
            if(files[0]->names[i] == NULL)
                 continue;
             else{
                 printf("    |--%s      [i#:%d size:%d]\n",  files[0]->names[i], files[0]->inode_index[i], InodeMap[files[0]->inode_index[i]].size);
                 if(files[Find_folder_index(files[0]->names[i])]->type == 0){
                     continue;
                 }
                 for(int j =1; j<MAX_PER_FOLDER; j++){
                     if(files[Find_folder_index(files[0]->names[i])]!=NULL){
                         if(files[Find_folder_index(files[0]->names[i])]->names[j]==NULL){
                             continue;
                         }
                         else{
                             printf("        |--%s      [i#:%d size:%d]\n",  files[Find_folder_index(files[0]->names[i])]->names[j], files[Find_folder_index(files[0]->names[i])]->inode_index[j], InodeMap[files[Find_folder_index(files[0]->names[i])]->inode_index[j]].size);
                             if(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])] == NULL){
                             }
                             if(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->type==0){
                                 continue;
                             }
                             for(int k =1; k<MAX_PER_FOLDER; k++){
                                 if(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]!=NULL){
                                     if(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k]==NULL)
                                         continue;
                                     else{
                                         printf("            |--%s      [i#:%d size:%d]\n",  files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k], files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->inode_index[k], InodeMap[files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->inode_index[k]].size);
                                         if(files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->type==0)
                                                 continue;
                                         for(int l =1; l<MAX_PER_FOLDER; l++){
                                             if(files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]!=NULL){
                                                 if(files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->names[l]==NULL)
                                                             continue;
                                                         else{
                                                             printf("                |--%s      [i#:%d size:%d]\n",  files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->names[j], files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->inode_index[j], InodeMap[files[Find_folder_index(files[Find_folder_index(files[Find_folder_index(files[0]->names[i])]->names[j])]->names[k])]->inode_index[j]].size);
                                                         }
                                                     }
                                        }
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

// print out all files
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
    //read free block list into memory
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
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
    //Checking the integrity of the inodes and the blocks
    fsck(disk);
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
                memset(textbuff,0, 31);
                entries[j]= buffer[j*32];
                fseek(disk, InodeMap[i].block_num[0]*512+j*32+1, SEEK_SET);
                fread(textbuff, 31, 1, disk);
                
                //printf("textbuffer:\n");
                //for(int s = 0;s < 31; s++){
                //    printf("%.2X ", (int)textbuff[s]);
                //}
                //printf("\n");
                
                strcpy(names[j].s , textbuff);
                sizecount++;
                if(strlen(names[j].s) == 0){
                    free(textbuff);
                    continue;
                }
                //printf("    entry:%d name:%s\n", entries[j], names[j].s);
                strcpy(InodeMap[buffer[j*32]].name , names[j].s);
                free(textbuff);
            }
           // printf("*** Loadfile:%s,%d, entry:%d%s,%d%s,%d%s,%d%s, sizecount:%d\n\n", names[0].s,type,entries[0],names[0].s,entries[1],names[1].s, entries[2], names[2].s,entries[3], names[3].s, sizecount);
            
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

// Load directories info from vdisk
struct filenode* LoadFiles(int type, int entries[] ,struct String* name, int size){
    struct filenode* file = (struct filenode*)malloc(sizeof(struct filenode));
    file->type = type;
    //printf("    copy size:%d\n", size);
    for(int i =0; i <size; i++){
        file->inode_index[i] = entries[i];
        file->names[i] = (char*)malloc(100);
        if(strlen(name[i].s)>0){
           strcpy(file->names[i],name[i].s);
            //printf("    Copy [%d] %s to filename, now it is %s\n",i, name[i].s, file->names[i] );
        }
        else{
            file->names[i] = NULL;
            //printf("    [%d]%s lenth=0\n",i, name[i].s);
        }
        
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

//update the inodeMap to the vdisk
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

// update inode-map
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

// update directories
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
    // All updates done, reset fsck status to FALSE;
    Set_fsck (disk, 0);
    printf("All updates written to disk, fsck_status=FALSE\n----------------------------\n\n");
}

void Set_fsck (FILE* disk, int fsck_indicator){
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    memset(buffer, 0, 512);
    memcpy(buffer, &fsck_indicator, sizeof(int));
    writeBlock(disk, 8, buffer);
    free(buffer);
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
    Set_fsck(disk, 1);  // Mark fsck status to start of Creation
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
void Writing(FILE* disk, char* pathname, unsigned char* content)
{
    Loading(disk);
    Set_fsck(disk, 2);  // Mark fsck status to start of Writing
    printf("Writing content in disk\n");
    int finode = Check_path(pathname);
    if(finode == -1){
        printf("Invalid Pathname!\n");
        return;
    }
    //check inode's 10 indirect blocks whether there is free ones
    int blocks_status = -1;
    for(int i=0; i<10; i++){
        if(InodeMap[finode].block_num[i]==0)
            blocks_status = 0;
    }
    if(blocks_status == -1){
        printf("Inode[%d] %s has no free indirect blocks. No writing\n", finode, InodeMap[finode].name);
        return;
    }
    
    int cur_block = 0;
    int remaing_size = (int)strlen((char*)content);
    InodeMap[finode].size += remaing_size;
    
    // Update the file size;
    InodeMap[0].size += remaing_size;
    for(int i = 1; i<MAX_FOLDER_NUM; i++){
        for(int j=1; j<MAX_PER_FOLDER; j++){
            if(files[i]!=NULL){
                if(files[i]->inode_index[j] == finode){
                    InodeMap[files[i]->inode_index[0]].size+= remaing_size;
                    //printf("    inode[%d]==%d %s +=%d\n",j, finode,InodeMap[files[i]->inode_index[0]].name, remaing_size);
                    for(int k = 1; k < MAX_FOLDER_NUM; k++){
                        for(int l = 1; l<MAX_PER_FOLDER; l++){
                            if(files[k]!=NULL){
                                if(files[k]->inode_index[l]==files[i]->inode_index[0]){
                                    InodeMap[files[k]->inode_index[0]].size+= remaing_size;
                                    //printf("    inode[%d]==%d %s +=%d\n",l,files[i]->inode_index[j], InodeMap[files[k]->inode_index[0]].name, remaing_size);
                                    for(int m =1; m<MAX_FOLDER_NUM; m++){
                                        for(int n = 1; n<MAX_PER_FOLDER; n++){
                                                                                   if(files[m]!=NULL){ if(files[m]->inode_index[n] == files[k]->inode_index[0]){
                                                //printf("    inode[%d]==%d %s +=%d\n",n, files[k]->inode_index[l],InodeMap[files[m]->inode_index[0]].name, remaing_size);
                                                                                       InodeMap[files[m]->inode_index[0]].size += remaing_size;
                                                                                   }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    printf("  write %d bytes to %s\n", remaing_size, pathname);
    unsigned char* buffer = (unsigned char*)malloc(512);
    int chunks =0;
    while(cur_block <10){
        if(InodeMap[finode].block_num[cur_block]==0){
            memset(buffer, 0, 512);
            if(remaing_size>512){
                memcpy(buffer, content+512*chunks, BLOCK_SIZE);
            }
            else{
                memcpy(buffer, content+512*chunks, remaing_size);
            }
            InodeMap[finode].block_num[cur_block] = Assign_afreeblock();
            // Crash scenario 2: occur just after blocks have been removed from the freelist;
            if(Crash_simulator ==2){
                printf("*** Crash [simulation scenario2].\n");
                exit(2);
            }
            
            printf("    write to location:%d\n", InodeMap[finode].block_num[cur_block]*512);
            writeBlock(disk,  InodeMap[finode].block_num[cur_block], buffer);
            // Crash scenario 1: occur after free blocks have been allocated to a file
            if(Crash_simulator ==1){
                printf("*** Crash [simulation scenario1].\n");
                exit(1);
            }
            
            remaing_size -= 512;
            chunks++;
            if(remaing_size<= 0){
                printf("All content has been written.\n\n");
                break;
            }
        }
        cur_block++;
    }
    free(buffer);
    if (cur_block>=10){
        printf("Inode[%d] %s run out of indirect blocks.\n", finode, InodeMap[finode].name);
    }
    Update_to_disk(disk);
}


// reading a file, load content into a buffer, then return it.
char* Reading (FILE* disk, char* pathname){
    Loading(disk);
    char*text= (char*)malloc(BLOCK_SIZE*10);
    memset(text, 0, sizeof(*text));
    printf("Reading content in disk\n");
    int finode = Check_path(pathname);
    if(finode == -1){
        printf("Invalid Pathname!\n");
        return NULL;
    }
    for(int i = 0; i <10; i++){
        if(InodeMap[finode].block_num[i]!=0){
            unsigned char* buffer;
            buffer = (unsigned char *) malloc(BLOCK_SIZE);
            readBlock(disk, InodeMap[finode].block_num[i], buffer);
            //printf("    read from position %d\n---%s\n", InodeMap[finode].block_num[i]*512, (char*)buffer);
            strcat(text, (char*)buffer);
            free(buffer);
        }
    }
    return text;
}

// deletion of files and directories
void Deleting (FILE* disk, char* pathname){
    Loading(disk);
    Set_fsck(disk, 3);  // Mark fsck status to start of Deletion
    printf("Start deletion\n");
    
    int finode = Check_path(pathname);
    if(finode == -1){
        printf("Invalid Pathname!\n");
        return;
    }
    if(InodeMap[finode].flag==1){
        for(int i = 1; i <MAX_PER_FOLDER; i++){
            if(files[Find_folder_index(InodeMap[finode].name)]->inode_index[i] !=0){
                printf("  %s is not empty, cannot be deleted!\n", InodeMap[finode].name);
                return;
            }
        }
        blocklist[InodeMap[finode].block_num[0]] = 0; //free its block
    }
    else{
        for(int i = 0; i <10; i++){
            blocklist[InodeMap[finode].block_num[i]] = 0;
        }
    }
    blocklist[InodeMap[finode].location/512]=0; //free the block for inode
    InodeMap[finode].location = 0;
    InodeMap[finode].flag = 0;
    memset(&InodeMap[finode].name[0],0,sizeof(InodeMap[finode].name));
    // remove its entry in parent directory
    for(int i=0; i<MAX_FOLDER_NUM; i++){
        for(int j=1; j<MAX_PER_FOLDER; j++){
            if(files[i]!=NULL){
                if(files[i]->inode_index[j] == finode){
                    printf("  %s size-%d=%d\n", files[i]->names[0], InodeMap[finode].size, InodeMap[files[i]->inode_index[0]].size-InodeMap[finode].size);
                    files[i]->inode_index[j] = 0;
                    files[i]->names[j] = NULL;
                    InodeMap[files[i]->inode_index[0]].size-= InodeMap[finode].size;
                    
                    for(int k=0; k<MAX_FOLDER_NUM; k++){
                        if(files[k]!=NULL){
                            for(int l=1; l<MAX_PER_FOLDER; l++){
                                if(files[k]->inode_index[l] == files[i]->inode_index[0] && files[i]->inode_index[0]!=0){
                                    printf("   %s size-%d=%d\n", files[k]->names[0], InodeMap[finode].size, InodeMap[files[k]->inode_index[0]].size-InodeMap[finode].size);
                                    InodeMap[files[k]->inode_index[0]].size-= InodeMap[finode].size;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    InodeMap[finode].size = 0;
    for(int i=0; i <MAX_FOLDER_NUM; i++){
        if(files[i]!=NULL){
            if(files[i]->inode_index[0]==finode)
                files[i] = NULL;
        }
    }

    printf("Done deletion.\n----------------------------\n");
    
    Update_to_disk(disk);
}


// return a free block#, return -1 if full
int Assign_afreeblock()
{
    for(int i=0; i<NUM_BLOCKS; i++){
        if(blocklist[i]==0){
            blocklist[i] = 1;
            return i;
        }
    }
    printf("Error: No free blocks available!\n");
    return -1;
}


// read content stored in a file into buffer, return 1 if successful, return 0 if fail
int read_extdata(char **result,char *fileName)
{
    struct stat fileInfo;
    FILE *filePointer;
    char *fileDate;

    if(fileName==NULL)
    {
        printf("NULL %s\n", fileName);
        return 0;
    }
    
    char* path = "../apps/";
    char* pathname = (char *) malloc(1 + strlen(path)+ strlen(fileName) );
    strcpy(pathname,path);
    strcat(pathname,fileName);
    printf("External source: %s\n", pathname);
    if(!(filePointer=fopen(pathname,"rb ")))
    {return 0;}

    stat(fileName,&fileInfo);

    fileDate=malloc(sizeof(char)*(fileInfo.st_size+1));

    fread(fileDate,sizeof(char),fileInfo.st_size,filePointer);

    fclose(filePointer);

    fileDate[fileInfo.st_size]=0;


    //printf("\n%s\n",fileDate);
    *result=fileDate;
    //printf("\n-------------\n");
    return 1;

}

void fsck(FILE* disk){
    unsigned char* buffer;
    buffer = (unsigned char *) malloc(BLOCK_SIZE);
    readBlock(disk, 8, buffer);
    Fsck_status = (int)buffer[0];
    free(buffer);
    if(Fsck_status == 0){
        printf("Sys_check: No fsck needed\n\n");
        return;
    }
    printf("\nSys_check: VDisk not properly ejected, File System Check running now.\n----------------------------\n");
    
    i_check(disk);
    d_check(disk);
    
    printf("Sys_check: Done!\n\n");
    Update_to_disk(disk);

    //Set_fsck(disk, 0);
    
}


void i_check(FILE* disk){
    // check block_list based on inodes
    for(int i =0; i<NUM_BLOCKS; i++){
        if(i<10)
            blocklist[i] = 1;
        else
            blocklist[i] = 0;
    }
    for(int i =0; i<NUM_INODES; i++){
        for(int j=0; j<10; j++){
            if(InodeMap[i].block_num[j]!=0){
                if(blocklist[ InodeMap[i].block_num[j] ]==0) // if block is in inodes, set it used
                    blocklist[ InodeMap[i].block_num[j] ]=1;
                else{
                    printf("ERROR: block[%d] in multiple inodes! Fix it manually\n", InodeMap[i].block_num[j]);
                    exit(9);
                }
            }
        }
    }
    // free_block_list now match inodes
    printf("  Done i_check.\n");
}

void d_check(FILE* disk){
    for(int i=0; i<NUM_INODES; i++){
        if(InodeMap[i].flag == 1){
            for(int j =1; j<MAX_PER_FOLDER; j++){
                if(InodeMap[i].block_num[j]!=0){
                    // if the entry file nolonger exists, then delete it
                    if( files[Find_folder_index(InodeMap[i].name)] == NULL ){
                        InodeMap[i].block_num[j] = 0;
                    }
                    else{
                        for(int k=1; k< MAX_PER_FOLDER; k++){
                            if(files[Find_folder_index(InodeMap[i].name)]->inode_index[k]==0){
                                files[Find_folder_index(InodeMap[i].name)]->inode_index[k] = InodeMap[i].num;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    printf("  Done d_check.\n");
}

// 0 indicates turn off crash_simulator; 1 for crash scenario1, 2 for scenario2.
void Set_simulator(int sim){
    Crash_simulator = sim;
    printf("Set Crash [scenario %d]\n", sim);
}
