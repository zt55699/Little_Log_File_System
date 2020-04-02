/* test10.c Shell app
 § a shell to interact with Vdisk
 § commands: mkdir
 */
#include "Shelltest.h"
#include "../io/File.h"
#include "../disk/disk.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
int current_folder = 0;
    // start in root directory
int SEEsh_map(char** args){
    Print_files();
    Print_structure();
    return 1;
}
int SEEsh_ls(char** args){
    Print_childs(current_folder);
    return 1;
}
int SEEsh_mkdir(char** args){
    if(args[1]==NULL){
        printf ("Directory name cannot be empty!\n");
        return 1;
    }
    FILE* disk = fopen(vdisk_path, "rb+");
    Load_Structure(disk);
    Createf(disk,Cur_path(current_folder), args[1], 1);
    fclose(disk);
    //Print_files();
    return 1;
}
int SEEsh_read(char** args){
    FILE* disk = fopen(vdisk_path, "rb");
    Load_Structure(disk);
    char* readfile = (char*) malloc(50);
    strcpy(readfile, Cur_path(current_folder));
    strcat(readfile, "/");
    strcat(readfile, args[1]);
    
    char* read = Reading(disk, readfile); //Reading() returns a char* of content
    if(read!=NULL){
        printf("%s\n", read);
        free(read);
    }
    free(readfile);
    fclose(disk);
    return 1;
}
int SEEsh_rm(char** args){
    FILE* disk = fopen(vdisk_path, "rb+");
    Load_Structure(disk);
    char* delefile = (char*) malloc(50);
    strcpy(delefile, Cur_path(current_folder));
    strcat(delefile, "/");
    strcat(delefile, args[1]);
    printf("Delete pathname:%s\n", delefile);
    Deleting(disk, delefile);
    
    free(delefile);
    fclose(disk);
    return 1;
}

int SEEsh_touch(char** args){
    FILE* disk = fopen(vdisk_path, "rb+");
    Load_Structure(disk);
    Createf(disk,Cur_path(current_folder), args[1], 0);
    fclose(disk);
    //Print_files();
    return 1;
}
int SEEsh_set(char** args){
    return 1;
}
int SEEsh_unset(char** args){
    return 1;
}

// write [filename]
int SEEsh_write(char** args){
    if(args[1]==NULL){
        printf("Aim filename cannot be empty!\n");
        return 1;
    }
    FILE* disk = fopen(vdisk_path, "rb+");
    Load_Structure(disk);
    char* writefile = (char*) malloc(50);
    strcpy(writefile, Cur_path(current_folder));
    strcat(writefile, "/");
    strcat(writefile, args[1]);
    
    printf(" Please type in the content you want to write:\n ");
    char* line = NULL;
    size_t buffersize = 0; //getline allocate memory    ssize_t
    getline(&line, &buffersize, stdin);
    Writing(disk, writefile, (unsigned char*)line);
    free (writefile);
    fclose(disk);
    return 1;
}

int SEEsh_num_builtins() {
    return sizeof(builtin_str)/ sizeof(char*);
}

int SEEsh_cd(char **args){
    FILE* disk = fopen(vdisk_path, "rb+");
    Load_Structure(disk);
    fclose(disk);
    int cdNUM = -1;
    if(args[1]==NULL || strcmp(args[1],"..") == 0){
        cdNUM = Check_parent(current_folder);
        current_folder = cdNUM;
        //printf("Current folder is %d\n", current_folder);
        return 1;
    }
    cdNUM = Check_child(current_folder, args[1]);
    if(cdNUM ==-1){
        printf("SEEsh: NO /%s find under current dir\n",args[1] );
        return -1;
    }
    if(cdNUM== -2){
        return -1;
    }
    current_folder = cdNUM;
    //printf("Current folder is %d\n", current_folder);
    return 1;
}

int SEEsh_help(char **args){
    int i;
    printf("Tong's CSC360 AS3 LLFS Shell test\n");
    printf("Input program names and arguments, and hit enter.\n");
    printf("Builtin commands are the followings:\n");
    
    for (i=0; i<SEEsh_num_builtins(); i++) {
        printf("   %s\n", builtin_str[i]);
    }
    printf("Use of write(): write [filename]\n");
    printf("vdisk initialized to /root directory.\n");
    printf("Use the man command for Linux's programs.\n");
    return 1;
}

int SEEsh_exit(char **args){
    return 0;
}

int SEEsh_pwd(char **args){
    printf ("%s\n", Cur_path(current_folder));
    return 1;
}


int SEEsh_execute(char **args){
    int i;
    if(args[0]==NULL){
        return 1;
    }
    if(args[0][0]==0){
        printf("SEEsh exits by CTRL+D \n");
        //possible memory leak here.
        return 0;
    }
    for(i = 0; i < SEEsh_num_builtins();i++) {
        if (strcmp(args[0], builtin_str[i]) == 0){
            //printf("run builtin\n");
            return (*builtin_fun[i]) (args);
        }
    }
    
    return SEEsh_launch(args);
}



int SEEsh_launch(char ** args){
    pid_t pid;
    int status;
    
    pid = fork();
    if(pid==0){
        //this is child
        if(execvp(args[0], args)== -1){ //variant of exec, v- vector, p-give name for search
            perror("SEEsh");
        }
        
        exit(EXIT_FAILURE);
    } else if(pid<0){
        //forking error
        perror("SEEsh");
    } else{
        do{
            waitpid(pid, &status, WUNTRACED);    //
        }while(!WIFEXITED(status)&& !WIFSIGNALED(status));
    }
    
    return 1;
}

char **SEEsh_split_line(char *line){
    int buffersize = SEEsh_TOK_BUFFERSIZE, position = 0;
    char** tokens = malloc(buffersize * sizeof(char*));
    char* token;
    if (!tokens){
        fprintf(stderr, "SEEsh: allocation error\n");
        exit (EXIT_FAILURE);
    }
    else if(line[0]== 0){
        char** ctrlD = malloc(buffersize * sizeof(char*));
        ctrlD[0] =  &line[0];
        return ctrlD;
    }
    token = strtok(line, SEEsh_TOK_DELIM);
    while (token!=NULL){
        tokens[position] = token;
        position ++;
        if(position >= buffersize){
            buffersize += SEEsh_TOK_BUFFERSIZE;
            tokens = realloc (tokens, buffersize*sizeof(char*));
            if(!tokens){
                fprintf(stderr, "SEEsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SEEsh_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

char *SEEsh_read_line(){
    char* line = NULL;
    size_t buffersize = 0; //getline allocate memory    ssize_t
    getline(&line, &buffersize, stdin);
    return line;
}

void SEEsh_loop(){
    FILE* disk = fopen(vdisk_path, "rb+");
    printf("Initialze the Vdisk? Y/N\n");
    char c =getchar();
    if (c=='Y')
        InitLLFS();
    Loading(disk);
    //loading disk inodes/map and block_list into memory
    fclose(disk);
    char* line;
    char** args;
    int status = 1;
    sleep(1);
    do{
        printf("? ");
        line = SEEsh_read_line();
        args = SEEsh_split_line(line);
        status = SEEsh_execute(args);
        
        free(line);
        free(args);
    } while (status);
    
}


int main(){

    //create flat files and create directories

    //Createf(disk,"/root", "readme.txt", 0);
    printf("Welcome to SEEsh!\n");
    SEEsh_loop();
    return EXIT_SUCCESS;
}

