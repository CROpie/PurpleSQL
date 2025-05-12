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

  if (!command->columnCount) {
    printf("No column count. Aborting. \n");
    hasPassedValidation = false;
  }

  if (!command->columnNames) {
    printf("No column names. Aborting. \n");
    hasPassedValidation = false;
  }

  if (!command->columnTypes) {
    printf("No column types. Aborting. \n");
    hasPassedValidation = false;
  }

  return hasPassedValidation;
}

Table* createTable(Command* command) {

  // Ensure command has all required fields
  if (!validateCreateTableCommand(command)) {
      return NULL;
  }      

  Table* table = malloc(sizeof(Table));

  table->schema.columnCount = command->columnCount;
  table->schema.columns = malloc(sizeof(ColumnSchema) * command->columnCount);
  
  for (int i = 0; i < command->columnCount; i++) {
    strcpy(table->schema.columns[i].name, command->columnNames[i]);
    table->schema.columns[i].type = command->columnTypes[i];

    printf("created type: %d\n", command->columnTypes[i]);
  }

  table->rowCount = 0;
  table->rows = NULL;

  return table;
}
