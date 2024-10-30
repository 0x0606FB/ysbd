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

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    

    CALL_BF(BF_CloseFile(fd1));

    return 0;
    //OK
}

HP_info* HP_OpenFile(char *file_name, int *file_desc){
    HP_info* hp_info;
    BF_Block* block = NULL;
    BF_ErrorCode error;

    BF_Block_Init(&block);
    
    error = BF_OpenFile(file_name, file_desc);
    if (error != BF_OK) {
      BF_PrintError(error); 
      BF_Block_Destroy(&block); 
      return NULL;
    }

    error = BF_GetBlock(*file_desc, 0, block);
    if (error != BF_OK) {
      BF_PrintError(error);
      BF_Block_Destroy(&block);
      return NULL;  // Return NULL in case of an error
    }

    void *data = BF_Block_GetData(block);
    hp_info = (HP_info*)data;


	
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


int HP_InsertEntry(int file_desc, HP_info* header_info, Record record) {
    BF_Block *block;
    BF_Block_Init(&block);

    int record_size = sizeof(Record);
    int max_records_in_block = 8;  // Max number of records per block
    void *data;

    // Handle case where this is the first entry
    if (header_info->last_block_id == 0) {
        // Allocate a new block since no blocks have been created yet
        CALL_BF(BF_AllocateBlock(file_desc, block));
        data = BF_Block_GetData(block);
        
        // Insert the new record into the fresh block
        memcpy(data, &record, record_size);
        
        // Update the header info
        header_info->last_block_id = 1;  // Set to the first block
        
        // Mark the new block as dirty and unpin it
    } else {
        // Try to access the last block
        CALL_BF(BF_GetBlock(file_desc, header_info->last_block_id, block));
        data = BF_Block_GetData(block);

        // Calculate how many records are currently in the last block
        int records_in_last_block = 0;

        // Scan through the block to count how many records are already written
        for (int i = 0; i < max_records_in_block; i++) {
            Record *rec_in_block = (Record *)(data + i * record_size);
            if (rec_in_block->id != 0) {
                records_in_last_block++;
            } else {
                break;  // Found an empty spot
            }
        }

        if (records_in_last_block < max_records_in_block) {
            // There is space in the current block, insert the record
            memcpy(data + records_in_last_block * record_size, &record, record_size);

        } else {
            // The last block is full, so we need to allocate a new block
            CALL_BF(BF_UnpinBlock(block));  // Unpin the full block

            // Allocate a new block
            CALL_BF(BF_AllocateBlock(file_desc, block));
            data = BF_Block_GetData(block);  // Get the new block's data space

            // Insert the new record into the fresh block
            memcpy(data, &record, record_size);

            // Update the header info with the new block ID
            header_info->last_block_id++;  // Increment for the next allocation
        }
    }
    
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));

    // Clean up
    BF_Block_Destroy(&block);

    // Return the block ID where the record was inserted
    return header_info->last_block_id; // Return the updated block ID
}





int HP_GetAllEntries(int file_desc, HP_info* header_info, int id) {
    BF_Block *block = NULL;
    BF_Block_Init(&block);
    int total_blocks = header_info->last_block_id + 1;  // Total blocks we may have to read
    int blocks_read = 0;  // To count the number of blocks we read during the search
    int records_found = 0; // To track if we found any records

    // Iterate through all blocks (excluding the metadata block 0)
    for (int block_num = 1; block_num < total_blocks; block_num++) {
        // Get the block
        CALL_BF(BF_GetBlock(file_desc, block_num, block));
        void *data = BF_Block_GetData(block);

        // Calculate the number of records in this block
        int records_in_block = BF_BLOCK_SIZE / sizeof(Record);
        Record *record = (Record *)data;

        // Check each record in the block
        for (int i = 0; i < records_in_block; i++) {
            if (record[i].id == id) {
                printRecord(record[i]);
                records_found++;
            }
        }

        // Unpin the block after reading it
        CALL_BF(BF_UnpinBlock(block));
        blocks_read++;
    }

    BF_Block_Destroy(&block);

    if (records_found > 0) {
        printf("Records found: %d\n", records_found);
        return blocks_read;
    } else {
        printf("No records found with id: %d\n", id);
        return -1; // Return -1 if no records are found
    }
}


