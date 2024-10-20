#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../include/bf.h"
#include "../include/hp_file.h"
#include "../include/record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return BF_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){
    int fd1;
    CALL_BF(BF_CreateFile(fileName));
    CALL_BF(BF_OpenFile(fileName, &fd1));

    BF_Block *block;
    BF_Block_Init(&block);
    
    CALL_BF(BF_AllocateBlock(fd1, block));
    void *data = BF_Block_GetData(block);
    HP_info* mdata = data;
    mdata->blockid = 50;
    CALL_BF(BF_UnpinBlock(block));


    for (u_int64_t i = 0; i < BF_BUFFER_SIZE-1; i++) {
      BF_Block_Init(&block);
      CALL_BF(BF_AllocateBlock(fd1, block));
      CALL_BF(BF_UnpinBlock(block));
    }

    CALL_BF(BF_CloseFile(fd1));

    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){
    HP_info* hpInfo;
    BF_Block* block;
    
    BF_Block_Init(&block);

    BF_OpenFile(fileName, file_desc);

    for (u_int64_t i = 0; i < BF_BUFFER_SIZE; i++) {
    
      BF_GetBlock(*file_desc, i, block);
      void *data = BF_Block_GetData(block);
      hpInfo = data;
      printf("blockid: %d \n", hpInfo->blockid);
    }
    return hpInfo;
}


int HP_CloseFile(int file_desc,HP_info* hp_info ){
    return -1;
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
    return -1;
}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    
    return -1;
}

