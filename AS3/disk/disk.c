#include <stdio.h>
#include <string.h>
#include "../io/File.h"
#include "disk.h"

void readBlock(FILE* disk, int blockNum, unsigned char* buffer){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}


void writeBlock(FILE* disk, int blockNum, unsigned char* data){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fwrite(data, BLOCK_SIZE, 1, disk); // this will overwrite
}

void init_buffer(unsigned char buffer[], int size)
{
    for (int i = 0; i < size; i++){
        buffer[i] = 0x00;
    }
}

void print_buffer(unsigned char buffer[], int size)
{
    printf("%04x: ", 0);
    for (int i = 1; i <= size; i++){
        printf("%02x ", buffer[i-1]);
        if( i % 8 == 0)
        {
            printf("\n");
            printf("%04x: ", i);
        }
    }
    printf("\n");
}

void set_block(unsigned char buffer[], int block_num)
{
    int index = block_num / 8;
    int bit_index = block_num % 8;
    
   // printf("SET Block[%d] Index:%d  Bit_Index:%d\n", block_num, index, bit_index);
    buffer[index] |= 1UL << bit_index;
}

void unset_block(unsigned char buffer[], int block_num)
{
    int index = block_num / 8;
    int bit_index = block_num % 8;
    
    printf("UNSET Block[%d] Index:%d  Bit_Index:%d\n", block_num, index, bit_index);
    buffer[index] &= ~(1UL << bit_index);
}

void init_FBbuffer(unsigned char buffer[], int size)
{
    for (int i = 0; i < size; i++){
        buffer[i] = 0x00;
    }
    //printf("Reserve first 10 blocks in block list. \n");
    for (int i = 0; i < 10; i++){
        set_block(buffer, i);
    }
}

void print_hexbuffer(unsigned char* buffer)
{
    for (int i=0;i<BLOCK_SIZE;i++)
       {
            printf("%.2X ", (int)buffer[i]);
            if (i % 4 == 3)
            {
                printf(" ");
            }

            // Display 16 bytes per line
            if (i % 16 == 15)
            {
                printf("\n");
            }
       }
       printf("\n");
}

