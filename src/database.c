#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"

bool validateCreateTableCommand(Command* command) {
  bool hasPassedValidation = true;
  if (!command->tableName) {
    printf("No table name. Aborting.\n");
    hasPassedValidation = false;
  }

  if (!command->c_numColPairs) {
    printf("No column count. Aborting. \n");
    hasPassedValidation = false;
  }

  if (!command->c_colPairs) {
    printf("No column definitions. Aborting. \n");
    hasPassedValidation = false;
  }

  return hasPassedValidation;
}

// convert string to column type
ColumnType convertStrToColumnType(char* colDef) {
  ColumnType type = COL_UNDEFINED;

  if (strncmp(colDef, "INT", 3) == 0) {
    type = COL_INT;
  }

  if (strncmp(colDef, "BOOL", 4) == 0) {
    type = COL_BOOL;
  }

  if (strncmp(colDef, "VARCHAR(255)", 12) == 0) {
    type = COL_STRING;
  }

  return type;
}

Table* createTable(Command* command) {

  // Ensure command has all required fields
  if (!validateCreateTableCommand(command)) {
      return NULL;
  }      

  Table* table = malloc(sizeof(Table));

  table->schema.columnCount = command->c_numColPairs;
  table->schema.columns = malloc(sizeof(ColumnSchema) * command->c_numColPairs);

  for (int i = 0; i < command->c_numColPairs; i++) {
    strcpy(table->schema.columns[i].name, command->c_colPairs[i].colName);

    ColumnType type = convertStrToColumnType(command->c_colPairs[i].colDef);

    table->schema.columns[i].type = type;
  }

  table->rowCount = 0;
  table->rows = NULL;

  return table;
}
