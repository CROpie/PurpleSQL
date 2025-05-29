#ifndef ENCODER_H
#define ENCODER_H

#include "database.h"

#define DB_DIR "data"
#define MAX_DIR 64
#define MAX_PATH 128

void serializeRowToPage(Table* table, Page* page, Row* row, int rowIndex);
Row* deserializeRowFromPage(Table* table, Page* page, int rowIndex);
void saveTablesMetadata(Tables* tables, char* dbName);
Tables* loadTablesMetadata(char* dbName);

#endif
