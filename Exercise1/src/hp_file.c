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

    BF_Block *block = NULL;
    BF_Block_Init(&block);

    CALL_BF(BF_AllocateBlock(fd1, block));

    HP_info *hp_info;
    void *data;
    data = BF_Block_GetData(block);

    hp_info = (HP_info*)data;
    hp_info->available_blocks = BF_BUFFER_SIZE;
    hp_info->last_block_id = 0;
    hp_info->file_records = 0;

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    

    CALL_BF(BF_CloseFile(fd1));

    return 0;
}

HP_info* HP_OpenFile(char *file_name, int *file_desc){
    HP_info* hpInfo;
    BF_Block* block = NULL;
    BF_ErrorCode error;

    BF_Block_Init(&block);
    
    error = BF_OpenFile(file_name, file_desc);
    if (error != BF_OK) {BF_PrintError(error); BF_Block_Destroy(&block); return NULL;}

    HP_info* hp_info;														
  	hp_info = (HP_info*)BF_Block_GetData(block);

    BF_Block_Destroy(&block);
	
	  return hp_info;
}


int HP_CloseFile(int file_desc, HP_info* hp_info){
    
    BF_Block *block = NULL;
    BF_Block_Init(&block);

    CALL_BF(BF_GetBlock(file_desc, 0, block));							            
    
    CALL_BF(BF_UnpinBlock(block));													
    BF_Block_Destroy(&block);														
    
    CALL_BF(BF_CloseFile(file_desc));												

	return 0;
}

int HP_InsertEntry(int file_desc, HP_info* hp_info, Record record){
    // BF_Block *block;
    // BF_Block_Init(&block);

    // int records_per_block = BF_BLOCK_SIZE / sizeof(Record);

    // if (hp_info->available_blocks > 0) {
    //   CALL_BF(BF_GetBlock(file_desc, hp_info->last_block_id, block)); 
    // } else {
    //   CALL_BF(BF_AllocateBlock(file_desc, block));

    //   hp_info->last_block_id += 1;
    //   hp_info->available_blocks -= 1;    
    // }

    // char *data = 


    // return -1;
}

int HP_GetAllEntries(int fd,HP_info* hp_info, int value){    

}

