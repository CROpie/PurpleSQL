#ifndef ENCODER_H
#define ENCODER_H

#include "database.h"

void serializeRowToPage(Table* table, Page* page, Row* row, int rowIndex);
Row* deserializeRowFromPage(Table* table, Page* page, int rowIndex);
void savePage(Table* table, Page* page);

#endif
