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
    
    CALL_BF(BF_CreateFile(fileName));           //Create and open a BF file
    CALL_BF(BF_OpenFile(fileName, &fd1));

    BF_Block *block = NULL;
    BF_Block_Init(&block);

    CALL_BF(BF_AllocateBlock(fd1, block));      //Allocate a block to store file's metadata

    HP_info *hp_info;
    void *data;
    data = BF_Block_GetData(block);            //Get block's data/address

    hp_info = (HP_info*)data;                       //Proceed to store metadata in block
    hp_info->available_blocks = BF_BUFFER_SIZE;
    hp_info->last_block_id = 0;

    BF_Block_SetDirty(block);                   //Set block to dirty, unpin it and close the file
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    

    CALL_BF(BF_CloseFile(fd1));

    return 0;
}

HP_info* HP_OpenFile(char *file_name, int *file_desc){
    HP_info* hp_info;
    BF_Block* block = NULL;
    BF_ErrorCode error;

    BF_Block_Init(&block);                  
    
    error = BF_OpenFile(file_name, file_desc);      //Open the (BF) file
    if (error != BF_OK) {                           //Handle errors manually as CALL_BF cannot be used in a non-int context
      BF_PrintError(error); 
      BF_Block_Destroy(&block); 
      return NULL;
    }

    error = BF_GetBlock(*file_desc, 0, block);      //Get metadata block in position 0
    if (error != BF_OK) {
      BF_PrintError(error);
      BF_Block_Destroy(&block);
      return NULL;  // Return NULL in case of an error
    }

    void *data = BF_Block_GetData(block);       //Get metadata 
    hp_info = (HP_info*)data;


	
	return hp_info;             //We have succesfully opened a file and retrieved its metadata
}


int HP_CloseFile(int file_desc, HP_info* hp_info){
    
    BF_Block *block = NULL;
    BF_Block_Init(&block);

    CALL_BF(BF_GetBlock(file_desc, 0, block));  //Get metadata block			            
    
    CALL_BF(BF_UnpinBlock(block));				//Unpin and destroy it			    						
    BF_Block_Destroy(&block);														
    
    CALL_BF(BF_CloseFile(file_desc));			//Close file				    					

    free(hp_info);
	return 0;
}


int HP_InsertEntry(int file_desc, HP_info* header_info, Record record) {
    BF_Block *block;
    BF_Block_Init(&block);

    int record_size = sizeof(Record);
    int max_records_in_block = BF_BLOCK_SIZE/record_size;  //Max number of records per block
    void *data;

    
    if (header_info->last_block_id == 0) {                  //If the last block we handled is the metadata block,
        CALL_BF(BF_AllocateBlock(file_desc, block));         //we need to allocate a new block since no blocks have been created yet
        data = BF_Block_GetData(block);
        
        memcpy(data, &record, record_size);
        
        
        header_info->last_block_id = 1;  //Update the header info, set last_block_id to the first block
        
    } else {
        CALL_BF(BF_GetBlock(file_desc, header_info->last_block_id, block));  //Try to access the latest block
        data = BF_Block_GetData(block);

        int records_in_last_block = 0;              //Calculate how many records are currently in the latest block

        for (int i = 0; i < max_records_in_block; i++) {                //Count how many records are already written on the block (can't be more than max_records_in_block)
            Record *rec_in_block = (Record *)(data + i * record_size);
            if (rec_in_block->id != 0) {
                records_in_last_block++;
            } else {
                break;                                  //Found a spot where a record can fit
            }
        }

        if (records_in_last_block < max_records_in_block) {             //If there is space in the current block, insert the record
            memcpy(data + records_in_last_block * record_size, &record, record_size);

        } else {                                                        //If the last block is full, we need to allocate a new block
            CALL_BF(BF_UnpinBlock(block));                      //Unpin full block

            CALL_BF(BF_AllocateBlock(file_desc, block));        //Allocate a new block
            data = BF_Block_GetData(block);                 //Get the new block's data space

            memcpy(data, &record, record_size);             

            
            header_info->last_block_id++;  //Update the header info with the new block ID
        }
    }
    
    BF_Block_SetDirty(block);           //Set block to dirty and unpin it
    CALL_BF(BF_UnpinBlock(block));  


    BF_Block_Destroy(&block);

    
    return header_info->last_block_id; //Return the block ID where the record was inserted
}





int HP_GetAllEntries(int file_desc, HP_info* header_info, int id) {
    BF_Block *block = NULL;
    BF_Block_Init(&block);
    int total_blocks;        //Total blocks to search
    BF_GetBlockCounter(file_desc, &total_blocks);

    int blocks_read = 0;  //To count the number of blocks we search
    int records_found = 0; //To count number of records with given id we found

    for (int block_num = 1; block_num < total_blocks; block_num++) {           //Search all blocks (excluding the metadata block 0)

        CALL_BF(BF_GetBlock(file_desc, block_num, block));
        void *data = BF_Block_GetData(block);

        
        int records_in_block = BF_BLOCK_SIZE / sizeof(Record);  
        Record *record = (Record *)data;

        for (int i = 0; i < records_in_block; i++) {       //Check each record in the block
            if (record[i].id == id) {                      //Check each record's id
                printRecord(record[i]);                    //If we found a target, we print it
                records_found++;                           //And increment the sum of records found
            }
        }

        CALL_BF(BF_UnpinBlock(block));  //Unpin block when done with it
        blocks_read++;                  
    }

    BF_Block_Destroy(&block);

    if (records_found > 0) {
        printf("Records found: %d\n", records_found);       //Print number of records found
        return blocks_read;                                 //Return number of blocks searched
    } else {
        printf("No records found with id: %d\n", id);
        return -1; //Return -1 if no records are found
    }
}


