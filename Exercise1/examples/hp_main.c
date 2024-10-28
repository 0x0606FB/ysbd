#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/bf.h"
#include "../include/hp_file.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define FILE_NAME "test.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);


  HP_CreateFile(FILE_NAME);
  int file_desc, dummy;

  HP_info* hp_info = HP_OpenFile(FILE_NAME, &file_desc);



  Record record;  
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    dummy = HP_InsertEntry(file_desc,hp_info, record);
  }

  // int blocks_num;
  // BF_GetBlockCounter(file_desc, &blocks_num);
  // void *data;
  // BF_Block *block;

  // for (int i = 1; i < blocks_num; ++i) {
  //   printf("Contents of Block %d\n\t",1);
  //   CALL_OR_DIE(BF_GetBlock(file_desc, 1, block));
  //   data = BF_Block_GetData(block);
  //   Record* rec = data;
  //   printRecord(rec[0]);
  //   printf("\t");
  //   printRecord(rec[1]);
  //   CALL_OR_DIE(BF_UnpinBlock(block));
  // }

  // printf("RUN PrintAllEntries\n");
  // int id = rand() % RECORDS_NUM;
  // printf("\nSearching for: %d",id);
  // HP_GetAllEntries(file_desc,hp_info, id);

  HP_CloseFile(file_desc,hp_info);
  BF_Close();
}
