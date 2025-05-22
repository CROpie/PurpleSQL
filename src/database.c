#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"

void freeTable(Table* table) {

  if (!table) return;

  if (table->name) free(table->name);

  if (table->schema.columns) {
    free(table->schema.columns);
  }

  for (int i = 0; i < table->rowCount; i++) {
    if (table->rows[i]->values) {
      free(table->rows[i]->values);
      free(table->rows[i]);
    }
  }

  if (table->rows) {
    free(table->rows);
  }

  free(table);
}

void freeSelection(Selection* selection) {
  for (int i = 0; i < selection->selectedRowCount; i++) {
    if (selection->selectedRows[i].values) free(selection->selectedRows[i].values);
  }

  if (selection->selectedRows) free(selection->selectedRows);
  if (selection) free(selection);
}

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

  table->name = malloc(sizeof(command->tableName) + 1);
  strcpy(table->name, command->tableName);
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

    newRow->isDeleted = false;

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
              // add a check to see if string got truncated / will be truncated, since it is not char* but char[len]
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

void printSelection(Table* table, Selection* selection, int columnCount, SelectColumnInfo* whereColumnInfo) {

  for (int rowIndex = 0; rowIndex < selection->selectedRowCount; rowIndex++) {

    printf("{ ");

    for (int colIndex = 0; colIndex < columnCount; colIndex++) {

      switch (whereColumnInfo[colIndex].columnType) {

        case COL_INT:
          printf("%d, ", selection->selectedRows[rowIndex].values[colIndex]->intValue);
          break;
        case COL_BOOL:
          printf(selection->selectedRows[rowIndex].values[colIndex]->boolValue ? "true, " : "false, ");
          break;
        case COL_STRING:
          printf("'%s', ", selection->selectedRows[rowIndex].values[colIndex]->stringValue);
          break;
        default:
          printf("Unrecognized column type...\n");
      }  
    }
    printf("}\n");
  }
}

Selection* selectColumns(Table* table, Command* command) {

  // if * take columns from table schema and add them to command
  if (command->s_all) {
    free(command->s_colNames);
    command->s_colNameCount = table->schema.columnCount;
    command->s_colNames = malloc(sizeof(char*) * table->schema.columnCount);
    for (int schemaColIndex = 0; schemaColIndex < table->schema.columnCount; schemaColIndex++) {
      command->s_colNames[schemaColIndex] = malloc(strlen(table->schema.columns[schemaColIndex].name) + 1);
      strncpy(command->s_colNames[schemaColIndex], table->schema.columns[schemaColIndex].name, strlen(table->schema.columns[schemaColIndex].name));
    }
  }

  SelectColumnInfo* selectColumnInfo = malloc(sizeof(SelectColumnInfo) * command->s_colNameCount);
  int selectColumnInfoCount = 0;
  
  // create an array of indexes + types for the selected columns
  for (int schemaColIndex = 0; schemaColIndex < table->schema.columnCount; schemaColIndex++) {
    for (int selectColIndex = 0; selectColIndex < command->s_colNameCount; selectColIndex++) {
      if (strcmp(table->schema.columns[schemaColIndex].name, command->s_colNames[selectColIndex]) == 0) {
        selectColumnInfo[selectColumnInfoCount].columnIndex = schemaColIndex;
        selectColumnInfo[selectColumnInfoCount].columnType = table->schema.columns[schemaColIndex].type;
        selectColumnInfoCount++;
        break;
      }
    }
  }

  // determine index and type of column used in where clause
  SelectColumnInfo* whereColumnInfo = NULL;
  Operand whereClauseOperand;

  if (command->s_whereClause) {
    whereColumnInfo = malloc(sizeof(SelectColumnInfo));

    for (int commandColIndex = 0; commandColIndex < command->s_colNameCount; commandColIndex++) {
      if (strcmp(command->s_colNames[commandColIndex], command->s_whereClause->w_column) == 0) {
        whereColumnInfo->columnIndex = commandColIndex;
        whereColumnInfo->columnType = table->schema.columns[commandColIndex].type;
        break;
      }
    }

    if (strcmp(command->s_whereClause->w_operator, "=") == 0) {
      whereClauseOperand = OP_EQ;
    }
  }

  // selection object: contains the rows which pass the filter
  Selection* selection = calloc(sizeof(Selection), 1);
  selection->selectedRowCount = 0;
  selection->selectCapacity = SELECT_CAPACITY;
  selection->selectedRows = calloc(sizeof(SelectedRow) * selection->selectCapacity, 1);

  for (int rowIndex = 0; rowIndex < table->rowCount; rowIndex++) {
    int skip = 0;

    if (table->rows[rowIndex]->isDeleted) continue;

    if (command->s_whereClause) {

      if (whereClauseOperand == OP_EQ) {
        char* endptr;
        int val = (int) strtol(command->s_whereClause->w_value, &endptr, 10);
        if (*endptr != '\0') {
          printf("Invalid input: could not convert %s to int\n", command->s_whereClause->w_value);
          continue;
        }

        if (table->rows[rowIndex]->values[whereColumnInfo->columnIndex].intValue != val) skip = 1;
      }
    }

    if (skip) continue;

    if (selection->selectedRowCount >= selection->selectCapacity) {
      int oldCapacity = selection->selectCapacity;
      selection->selectCapacity *= 2;
      selection->selectedRows = realloc(selection->selectedRows, selection->selectCapacity * sizeof(SelectedRow));
    }

    selection->selectedRows[selection->selectedRowCount].values = malloc(sizeof(Value*) * command->s_colNameCount);

    // iterate usedSchemaIndex array
    // each int in the array is the position in table->rows[N]->values[] array
    for (int usedSchemaIndex = 0; usedSchemaIndex < command->s_colNameCount; usedSchemaIndex++) {
      selection->selectedRows[selection->selectedRowCount].values[usedSchemaIndex] = &table->rows[rowIndex]->values[selectColumnInfo[usedSchemaIndex].columnIndex];
    }
    selection->selectedRowCount++;
  }

  printSelection(table, selection, command->s_colNameCount, selectColumnInfo);

  free(selectColumnInfo);
  if (whereColumnInfo) free(whereColumnInfo);

  return selection;
}