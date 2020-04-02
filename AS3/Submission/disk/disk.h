#include <stdio.h>
#include <string.h>
#include "../io/File.h"

void readBlock(FILE* disk, int blockNum, unsigned char* buffer);

void writeBlock(FILE* disk, int blockNum, unsigned char* data);

void init_buffer(unsigned char buffer[], int size);

void print_buffer(unsigned char buffer[], int size);

void set_block(unsigned char buffer[], int block_num);

void unset_block(unsigned char buffer[], int block_num);

void init_FBbuffer(unsigned char buffer[], int size);

void print_hexbuffer(unsigned char* buffer);


