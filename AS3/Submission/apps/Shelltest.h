/* test10.h Shell app
 § a shell to interact with Vdisk
 § commands: mkdir
 */
#ifndef SHELLTEST_H
#define SHELLTEST_H
#include "../io/File.h"
#include "../disk/disk.h"
#include <unistd.h>
#include <signal.h>

#define SEEsh_TOK_BUFFERSIZE 512
#define SEEsh_TOK_DELIM " ,\t\r\n\a"
extern char **environ;
char** SEEsh_split_line(char*);
char* SEEsh_read_line(void);
void SEEsh_loop(void);
int SEEsh_launch(char**);

//builtin commands
char* builtin_str[] = {"cd", "help", "exit", "pwd", "mkdir", "touch", "write", "read", "rm", "ls","set", "unset"};
int SEEsh_cd(char** args);
int SEEsh_help(char** args);
int SEEsh_exit(char** args);
int SEEsh_pwd(char** args);
int SEEsh_mkdir(char** args);
int SEEsh_touch(char** args);
int SEEsh_write(char** args);
int SEEsh_read(char** args);
int SEEsh_rm(char** args);
int SEEsh_ls(char** args);
int SEEsh_set(char** args);
int SEEsh_unset(char** args);

int (*builtin_fun[]) (char **) = {
    &SEEsh_cd,
    &SEEsh_help,
    &SEEsh_exit,
    &SEEsh_pwd,
    &SEEsh_mkdir,
    &SEEsh_touch,
    &SEEsh_write,
    &SEEsh_read,
    &SEEsh_rm,
    &SEEsh_ls,
    &SEEsh_set,
    &SEEsh_unset
};


#endif
