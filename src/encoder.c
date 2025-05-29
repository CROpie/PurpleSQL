#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/stat.h> // mkdir
#include <unistd.h> // rmdir
#include <sys/types.h>
#include <errno.h>

#include "database.h"
#include "encoder.h"

void ensureDirExists(char* dirName) {
  struct stat st = {0};

  if (stat(dirName, &st) == -1) {
    if (mkdir(dirName, 0755) != 0) {
      printf("Failed to create directory '%s': %s\n", dirName, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
}

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

void savePage(Page* page, char* tableName, int pageNum) {

  char dir[MAX_DIR];
  snprintf(dir, sizeof(dir), DB_DIR "/%s", tableName);
  ensureDirExists(dir);

  char path[MAX_PATH];
  snprintf(path, sizeof(path), "%s/%s.db", dir, tableName);

  // char filename[MAX_TABLE_NAME + 3];
  // snprintf(filename, sizeof(filename), "%s.db", tableName);
  FILE* fp = fopen(path, "rb+");
  if (!fp) {
    fp = fopen(path, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  long offset = pageNum * sizeof(Page);
  fseek(fp, offset, SEEK_SET);

  fwrite(page, sizeof(Page), 1, fp);
  fclose(fp);
}

Page* loadPage(char* tableName, int pageNum) {

  char dir[MAX_DIR];
  snprintf(dir, sizeof(dir), DB_DIR "/%s", tableName);
  ensureDirExists(dir);

  char path[MAX_PATH];
  snprintf(path, sizeof(path), "%s/%s.db", dir, tableName);

  // char filename[MAX_TABLE_NAME + 3];
  // snprintf(filename, sizeof(filename), "%s.db", tableName);
  FILE* fp = fopen(path, "rb+");
  if (!fp) {
    fp = fopen(path, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  Page* page = calloc(sizeof(Page), 1);

  long offset = pageNum * sizeof(Page);
  fseek(fp, offset, SEEK_SET);

  fread(page, sizeof(Page), 1, fp);
  fclose(fp);

  return page;
}

void saveTableMetadata(Table* table) {

  char dir[MAX_DIR];
  snprintf(dir, sizeof(dir), DB_DIR "/%s", table->name);
  ensureDirExists(dir);

  char path[MAX_PATH];
  snprintf(path, sizeof(path), "%s/%s.meta", dir, table->name);

  // char filename[MAX_TABLE_NAME + 5];
  // snprintf(filename, sizeof(filename), "%s.meta", table->name);

  FILE* fp = fopen(path, "rb+");

  if (!fp) {
    fp = fopen(path, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  fwrite(&table->rowCount, sizeof(int), 1, fp);
  fwrite(&table->pageCount, sizeof(int), 1, fp);
  fwrite(&table->pageCapacity, sizeof(int), 1, fp);
  fwrite(&table->schema.columnCount, sizeof(int), 1, fp);
  fwrite(&table->schema.rowsPerPage, sizeof(int), 1, fp);
  for (int i = 0; i < table->schema.columnCount; i++) {
    fwrite(&table->schema.columns[i], sizeof(ColumnSchema), 1, fp);
  }
  fclose(fp);
}

void saveTablesMetadata(Tables* tables, char* dbName) {

  ensureDirExists(DB_DIR);

  char path[MAX_PATH];
  snprintf(path, sizeof(path), DB_DIR "/%s", dbName);

  FILE* fp = fopen(path, "rb+");

  if (!fp) {
    fp = fopen(path, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  fwrite(&tables->tableCount, sizeof(int), 1, fp);
  fwrite(&tables->tablesCapacity, sizeof(int), 1, fp);

  for (int i = 0; i < tables->tableCount; i++) {
    char paddedName[MAX_TABLE_NAME] = {0};
    strncpy(paddedName, tables->tableList[i]->name, MAX_TABLE_NAME - 1);
    paddedName[MAX_TABLE_NAME - 1] = '\0';
    fwrite(paddedName, sizeof(char), MAX_TABLE_NAME, fp);

    saveTableMetadata(tables->tableList[i]);
    for (int j = 0; j < tables->tableList[i]->pageCount + 1; j++) {
      if (!tables->tableList[i]->pages[j]) continue;
      savePage(tables->tableList[i]->pages[j], tables->tableList[i]->name, j);
    }
  }
  fclose(fp);
}

Table* loadTableMetadata(char* tableName) {

  char dir[MAX_DIR];
  snprintf(dir, sizeof(dir), DB_DIR "/%s", tableName);
  ensureDirExists(dir);

  char path[MAX_PATH];
  snprintf(path, sizeof(path), "%s/%s.meta", dir, tableName);

  FILE* fp = fopen(path, "rb+");

  if (!fp) {
    fp = fopen(path, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  Table* table = calloc(sizeof(Table), 1);
  table->name = calloc(sizeof(strlen(tableName)), 1);
  strcpy(table->name, tableName);

  fread(&table->rowCount, sizeof(int), 1, fp);
  fread(&table->pageCount, sizeof(int), 1, fp);
  fread(&table->pageCapacity, sizeof(int), 1, fp);
  fread(&table->schema.columnCount, sizeof(int), 1, fp);
  fread(&table->schema.rowsPerPage, sizeof(int), 1, fp);
  table->schema.columns = calloc(sizeof(ColumnSchema) * table->schema.columnCount, 1);
  for (int i = 0; i < table->schema.columnCount; i++) {
    fread(&table->schema.columns[i], sizeof(ColumnSchema), 1, fp);
  }
  fclose(fp);
  return table;
}

Tables* loadTablesMetadata(char* dbName) {

  // create root directory for data
  ensureDirExists(DB_DIR);

  // DB_DIR "/%s" => "db/tableName"
  char path[MAX_PATH];
  snprintf(path, sizeof(path), DB_DIR "/%s", dbName);

  FILE* fp = fopen(path, "rb+");

  if (!fp) {
    fp = fopen(path, "wb+");
    if (!fp) {
      printf("Unable to open file.\n");
      exit(EXIT_FAILURE);
    }
  }

  Tables* tables = calloc(sizeof(Tables), 1);

  if (!fread(&tables->tableCount, sizeof(int), 1, fp)) {
    tables->tableCount = 0;
  }
  if (!fread(&tables->tablesCapacity, sizeof(int), 1, fp)) {
    tables->tablesCapacity = TABLE_CAPACITY;
  }

  tables->tableList = calloc(sizeof(tables->tablesCapacity), 1);

  for (int i = 0; i < tables->tableCount; i++) {
    char tableName[MAX_TABLE_NAME];
    fread(&tableName, sizeof(char), MAX_TABLE_NAME, fp);
    Table* table = loadTableMetadata(tableName);
    table->pages = calloc(sizeof(Page*) * table->pageCapacity, 1);

    // REMOVE THIS WHEN LAZY LOADING PAGES
    for (int j = 0; j < table->pageCount + 1; j++) {
      table->pages[j] = loadPage(table->name, j);
    }

    tables->tableList[i] = table;
  }
  fclose(fp);
  return tables;
}

int deleteTableData(char* tableName) {
  char dir[MAX_DIR];
  snprintf(dir, sizeof(dir), DB_DIR "/%s", tableName);

  char metaPath[MAX_PATH];
  snprintf(metaPath, sizeof(metaPath), "%s/%s.meta", dir, tableName);

  char dataPath[MAX_PATH];
  snprintf(dataPath, sizeof(dataPath), "%s/%s.db", dir, tableName);

  if (remove(metaPath) != 0) return -1;

  if (remove(dataPath) != 0) return -2;
  
  if (rmdir(dir) != 0) return -3;

  return 0;
}