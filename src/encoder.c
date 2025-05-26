#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"

// rowIndex is index of row within a given page
void serializeRowToPage(Table* table, Page* page, Row* row, int rowIndex) {
    size_t rowSize = sizeof(Row) + sizeof(Value) * table->schema.columnCount;
    size_t offset = rowIndex * rowSize;

    // row metadata
    memcpy(&page->data[offset], row, rowSize);


}

Row* deserializeRowFromPage(Table* table, Page* page, int rowIndex) {
    size_t rowSize = sizeof(Row) + sizeof(Value) * table->schema.columnCount;
    size_t offset = rowIndex * rowSize;

    Row* row = calloc(rowSize, 1);
    memcpy(row, &page->data[offset], rowSize);
    return row;
}

void savePage(Table* table, Page* page) {
  FILE* fp = fopen("myFile.db", "rb+");
  if (!fp) {
    fp = fopen("myFile.db", "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }
  fwrite(page, sizeof(Page), 1, fp);
  fclose(fp);
}