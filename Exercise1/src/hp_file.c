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
    hp_info->last_block = block;
    hp_info->available_blocks = BF_BUFFER_SIZE;
    hp_info->last_block_id = 0;

    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    

    CALL_BF(BF_CloseFile(fd1));

    return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){
    HP_info* hpInfo;
    BF_Block* block;
    BF_ErrorCode error;

    BF_Block_Init(&block);

    BF_OpenFile(fileName, file_desc);
    error =  BF_OpenFile(fileName, file_desc);								// Open the file that was created earlier
  	if( error != BF_OK) { BF_PrintError(error); exit(EXIT_FAILURE); }

    HP_info* hp_info;														
  	hp_info = (HP_info*)BF_Block_GetData(block);

    BF_Block_Destroy(&block);
	
	  return hp_info;
}


int HP_CloseFile(int file_desc, HP_info* hp_info ){
    BF_Block *block = NULL;
    BF_Block_Init(&block);

    CALL_BF(BF_GetBlock(file_desc, 0, block));							            
    
    CALL_BF(BF_UnpinBlock(block));													
    BF_Block_Destroy(&block);														
    
    CALL_BF(BF_CloseFile(file_desc));												

	return 0;;
}

int HP_InsertEntry(int file_desc, HP_info* hp_info, Record record){
    BF_Block *block;
    BF_Block_Init(&block);
    
    return -1;

}

int HP_GetAllEntries(int fd,HP_info* hp_info, int value){    
  HP_block_info* block_info;
  Record* record;
	
  BF_Block *block = NULL;                                      					
  BF_Block_Init(&block);
  CALL_BF(BF_AllocateBlock(fd, block)); 
    
  int blocks_num;																// Get number of blocks in file
  CALL_BF(BF_GetBlockCounter(fd, &blocks_num));

	int blocks_read = 0;															// No blocks read yet
    
	for(int i = 0; i < blocks_num; i++){

    CALL_BF(BF_GetBlock(fd, i, block));									// Get block i in file
        
		char* block_data = BF_Block_GetData(block);										// Get it's data
        
		block_info = (HP_block_info*)(data + BF_BLOCK_SIZE -sizeof(block_info));
    record =(Record*)(data);													// First record of block is at start of block
        
		for(int j = 0; j < block_info; j++) {						// For each record inside the block
      if(record[j].id == value) {												// If value is found
        blocks_read = i+1;                                                  // Number of blocks read until last record has been found
        printRecord(record[j]);                                             // !!we are not talking about the total number of blocks in the file that will be searched!!
      }	
          
    }
		
		CALL_BF(BF_UnpinBlock(block));
  		
  }

	BF_Block_Destroy(&block);  
  return blocks_read;
}

