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
  table->rowCapacity = ROW_CAPACITY;
  table->rows = malloc(sizeof(Row*) * table->rowCapacity);
  table->rowCount = 0;

  for (int i = 0; i < command->c_numColPairs; i++) {
    strcpy(table->schema.columns[i].name, command->c_colPairs[i].colName);

    ColumnType type = convertStrToColumnType(command->c_colPairs[i].colDef);

    table->schema.columns[i].type = type;
  }

  // table->rowCount = 0;
  // table->rows = NULL;

  return table;
}

void insertRecord(Table* table, Command* command) {

 if (command->i_numColNames > table->schema.columnCount) {
  printf("Schema mismatch: to many columns entered.\n");
  return;
 }

  int numCols = command->i_numColNames;

  // fill up rows 1 by 1
  for (int colValueRowIndex = 0; colValueRowIndex < command->i_numValueRows; colValueRowIndex++) {

    Row* newRow = malloc(sizeof(Row));

    // malloc a row of values
    newRow->values = malloc(sizeof(Value) * numCols);

    // iterate over i_colNames - need to determine the types
    for (int colNameIndex = 0; colNameIndex < numCols; colNameIndex++) {

      // attempt to find matching column
      for (int schemaColNameIndex = 0; schemaColNameIndex < numCols; schemaColNameIndex++) {

        // search for match
        if (!strcmp(command->i_colNames[colNameIndex], table->schema.columns[schemaColNameIndex].name) == 0) continue;

        char* str = command->i_colValueRows[colValueRowIndex][colNameIndex];

        // coerce value from string into correct type
        switch (table->schema.columns[schemaColNameIndex].type) {
          case COL_INT:
            char* endptr;
            int val = (int) strtol(str, &endptr, 10);
            if (*endptr != '\0') {
              printf("Invalid input: could not convert %s to int\n", str);
            }
            newRow->values[colNameIndex].intValue = val;
            break;
          case COL_BOOL:
            if (strcmp(str, "true") == 0) {
              newRow->values[colNameIndex].boolValue = true;
            } else if (strcmp(str, "false") == 0) {
              newRow->values[colNameIndex].boolValue = false;
            } else {
              printf("Invalid input: could not convert %s to bool\n", str);
            }
            break;
          case COL_STRING:
              strcpy(newRow->values[colNameIndex].stringValue, str);
              // add a check to see if string got truncated / will be truncated
            break;
          default:
            printf("unrecognized type");
        }
    }
  }

  // reallocate if needed
  if (table->rowCount >= table->rowCapacity) {
    int oldCapacity = table->rowCapacity;
    table->rowCapacity *= 2;
    table->rows = realloc(table->rows, table->rowCapacity * sizeof(Row));
  }

  table->rows[table->rowCount++] = newRow;

 }
}

void printAll(Table* table) {

  for (int rowIndex = 0; rowIndex < table->rowCount; rowIndex++) {

    printf("{ ");

    for (int colIndex = 0; colIndex < table->schema.columnCount; colIndex++) {

      // printf("colIndex: %d type: %d\n", colIndex, table->schema.columns[colIndex].type);

      switch (table->schema.columns[colIndex].type) {
        case COL_INT:
          printf("%d, ", table->rows[rowIndex]->values[colIndex].intValue);
          break;
        case COL_BOOL:
          printf(table->rows[rowIndex]->values[colIndex].boolValue ? "true, " : "false, ");
          break;
        case COL_STRING:
          printf("'%s', ", table->rows[rowIndex]->values[colIndex].stringValue);
          break;
        default:
          printf("Unrecognized column type...\n");
      }  
    }
    printf("}\n");
  }
}

void selectColumns(Table* table, Command* command) {
  if (command->s_all) {
    printAll(table);
  }
}