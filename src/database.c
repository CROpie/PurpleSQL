#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"
#include "encoder.h"

void freeTable(Table* table) {

  if (!table) return;

  if (table->name) free(table->name);

  if (table->pages) {
    int pagesCount = (table->rowCount / table->schema.rowsPerPage) + 1;
    for (int i = 0; i < pagesCount; i++) {
      free(table->pages[i]);
    }
    free(table->pages);
  }

  if (table->schema.columns) {
    free(table->schema.columns);
  }

  free(table);
}

void freeTables(Tables* tables) {
  for (int i = 0; i < tables->tableCount; i++) {
    freeTable(tables->tableList[i]);
  }
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

  char errMsg[1024];
  errMsg[0] = '\0';

  if (!command->tableName) {
    strcat(errMsg, "Error: No table name.\n");
    hasPassedValidation = false;
  }

  if (!command->c_numColPairs) {
    strcat(errMsg, "Error: No columns.\n");
    hasPassedValidation = false;
  }

  if (!command->c_colPairs) {
    strcat(errMsg, "Error: No column definitions.\n");
    hasPassedValidation = false;
  }

  if (!hasPassedValidation) {
    command->e_message = calloc(strlen(errMsg) + 1, 1);
    strcpy(command->e_message, errMsg);
    command->type = CMD_ERROR;
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

Table* createTable(Tables* tables, Command* command) {

  // Ensure command has all required fields 
  if (!validateCreateTableCommand(command)) {
      return NULL;
  }

  // find table based on tableName
  for (int i = 0; i < tables->tableCount; i++) {
    if (!tables->tableList[i]) continue;
    
    if (strcmp(tables->tableList[i]->name, command->tableName) == 0) {
      command->e_message = strdup("Error: table already exists.\n");
      command->type = CMD_ERROR;
      return NULL;
    }
  }

  Table* table = calloc(sizeof(Table), 1);

  table->name = calloc(sizeof(command->tableName) + 1, 1);
  strcpy(table->name, command->tableName);
  table->schema.columnCount = command->c_numColPairs;
  table->schema.columns = calloc(sizeof(ColumnSchema) * command->c_numColPairs, 1);
  table->rowCount = 0;

  for (int i = 0; i < command->c_numColPairs; i++) {
    strcpy(table->schema.columns[i].name, command->c_colPairs[i].colName);

    ColumnType type = convertStrToColumnType(command->c_colPairs[i].colDef);

    if (type == COL_UNDEFINED) {
      command->e_message = strdup("Error: Unknown column type.\n");
      command->type = CMD_ERROR;
      freeTable(table);
      return NULL;
    }

    table->schema.columns[i].type = type;
  }

  table->pageCount = 0;
  table->pageCapacity = PAGE_CAPACITY;

  // sizeof(Row) does _not_ take into account the variable array value[] 
  size_t rowSize = sizeof(Row) + sizeof(Value) * table->schema.columnCount;
  table->schema.rowsPerPage = PAGE_SIZE / rowSize;

  if (table->schema.rowsPerPage == 0) {
      command->e_message = strdup("Error: Too many columns.\n");
      command->type = CMD_ERROR;
      freeTable(table);
      return NULL;
  }

  table->pages = calloc(sizeof(Page*) * table->pageCapacity, 1);

  return table;
}

bool insertRecord(Tables* tables, Command* command) {

  Table* table = NULL;

  // find table based on tableName 
  for (int i = 0; i < tables->tableCount; i++) {
    if (strcmp(tables->tableList[i]->name, command->tableName) == 0) {
      table = tables->tableList[i];
      break;
    }
  }

  if (!table) {
    command->e_message = strdup("Error: table not found.\n");
    command->type = CMD_ERROR;
    return false;
  }

 // check that all columns in command exist in table schema
 for (int i = 0; i < command->i_numColNames; i++) {
  bool found = false;

  for (int j = 0; j < table->schema.columnCount; j++) {
    if (strcmp(command->i_colNames[i], table->schema.columns[j].name) == 0) found = true;
  }

  if (!found) {
    command->e_message = strdup("Error: unrecognized column entered.\n");
    command->type = CMD_ERROR;
    return false;
  }

 }

  // ensure no column duplicates
 for (int i = 0; i < command->i_numColNames; i++) {
  bool found = false;

  for (int j = i + 1; j < command->i_numColNames; j++) {
    if (strcmp(command->i_colNames[i], command->i_colNames[j]) == 0) found = true;
  }

  if (found) {
    command->e_message = strdup("Error: duplicate column detected.\n");
    command->type = CMD_ERROR;
    return false;
  }

 }

  int numCols = command->i_numColNames;



  // fill up rows 1 by 1
  for (int colValueRowIndex = 0; colValueRowIndex < command->i_numValueRows; colValueRowIndex++) {

    if (table->rowCount >= table->schema.rowsPerPage * table->pageCapacity) {
      int oldCapacity = table->pageCapacity;
      table->pageCapacity *= 2;
      table->pages = realloc(table->pages, table->pageCapacity * sizeof(Page*));
    }

    int pageNumber = table->rowCount / table->schema.rowsPerPage;


    if (!table->pages[pageNumber]) {
      table->pages[pageNumber] = calloc(sizeof(Page), 1);
    }

    Row* newRow = calloc(sizeof(Row) + sizeof(Value) * numCols, 1);

    newRow->isDeleted = false;

    // iterate over i_colNames - need to determine the types
    for (int colNameIndex = 0; colNameIndex < numCols; colNameIndex++) {

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
              command->e_message = strdup("Error: Value not int.\n");
              command->type = CMD_ERROR;
              break;
            }
            newRow->values[colNameIndex].intValue = val;
            break;
          case COL_BOOL:
            if (strcmp(str, "true") == 0) {
              newRow->values[colNameIndex].boolValue = true;
            } else if (strcmp(str, "false") == 0) {
              newRow->values[colNameIndex].boolValue = false;
            } else {
              command->e_message = strdup("Error: Value not bool.\n");
              command->type = CMD_ERROR;
            }
            break;
          case COL_STRING:
            if (strlen(str) >= sizeof(newRow->values[colNameIndex].stringValue) - 1) {
              command->e_message = strdup("Error: String is too long.\n");
              command->type = CMD_ERROR;
            } else {
              strcpy(newRow->values[colNameIndex].stringValue, str);
            }
            break;
          default:
              command->e_message = strdup("Error: Unrecognized type.\n");
              command->type = CMD_ERROR;
        }
        if (command->type == CMD_ERROR) {
          free(newRow);
          // other things need to be freed here too...
          return false;
        }
    }
  }

  int rowOffset = table->rowCount % table->schema.rowsPerPage;
  serializeRowToPage(table, table->pages[pageNumber], newRow, rowOffset);
  table->rowCount++;

 }

   return true;
}

void printSelection(Table* table, Selection* selection, int columnCount, SelectColumnInfo* selectColumnInfo) {

  for (int rowIndex = 0; rowIndex < selection->selectedRowCount; rowIndex++) {

    printf("{ ");

    for (int colIndex = 0; colIndex < columnCount; colIndex++) {

      switch (selectColumnInfo[colIndex].columnType) {

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

Selection* selectColumns(Tables* tables, Command* command) {

  Table* table = NULL;

  // find table based on tableName
  for (int i = 0; i < tables->tableCount; i++) {
    if (strcmp(tables->tableList[i]->name, command->tableName) == 0) {
      table = tables->tableList[i];
      break;
    }
  }

  if (!table) {
    command->e_message = strdup("Error: table not found.\n");
    command->type = CMD_ERROR;
    return NULL;
  } 



  // printf("s_all: %d\n", command->s_all);

  // if * take columns from table schema and add them to command
  if (command->s_all) {
    free(command->s_colNames);
    command->s_colNameCount = table->schema.columnCount;
    command->s_colNames = calloc(sizeof(char*) * table->schema.columnCount, 1);
    for (int schemaColIndex = 0; schemaColIndex < table->schema.columnCount; schemaColIndex++) {
      command->s_colNames[schemaColIndex] = calloc(strlen(table->schema.columns[schemaColIndex].name), 1);
      strncpy(command->s_colNames[schemaColIndex], table->schema.columns[schemaColIndex].name, strlen(table->schema.columns[schemaColIndex].name));
    }
  }

     // check that all columns in command exist in table schema 
 for (int i = 0; i < command->s_colNameCount; i++) {
  bool found = false;

  for (int j = 0; j < table->schema.columnCount; j++) {
    if (strcmp(command->s_colNames[i], table->schema.columns[j].name) == 0) found = true;
  }

  if (!found) {
    command->e_message = strdup("Error: unrecognized column entered.\n");
    command->type = CMD_ERROR;
    return NULL;
  }
 }

  // ensure no column duplicates
 for (int i = 0; i < command->s_colNameCount; i++) {
  bool found = false;

  for (int j = i + 1; j < command->s_colNameCount; j++) {
    if (strcmp(command->s_colNames[i], command->s_colNames[j]) == 0) found = true;
  }

  if (found) {
    command->e_message = strdup("Error: duplicate column detected.\n");
    command->type = CMD_ERROR;
    return NULL;
  }
 }

 // check where column exists


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

    bool found = false;
    for (int commandColIndex = 0; commandColIndex < command->s_colNameCount; commandColIndex++) {

      if (strcmp(command->s_colNames[commandColIndex], command->s_whereClause->w_column) == 0) {
        whereColumnInfo->columnIndex = commandColIndex;
        whereColumnInfo->columnType = table->schema.columns[commandColIndex].type;
        found = true;
        break;
      }
    }

    if (!found) {
      command->e_message = strdup("Error: unrecognized where column entered.\n");
      command->type = CMD_ERROR;
      return NULL;
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

  /* One big buffer for all rows */
  /*
  Perhaps loop through pages, malloc + memCpy entire page, free page at the end (after copying the selected rows) ?


  size_t rowSize = sizeof(Row) + sizeof(Value) * table->schema.columnCount;
  Row* buffer = calloc(rowSize * table->rowCount);

  determine number of pages
  int pageCount = (table->rowCount / table->schema.rowsPerPage) + 1;

  for (int pageNum = 0; pageNum < pageCount; pageNum++) {
    if (!table->pages[pageNumber]) {
      table->pages[pageNumber] = calloc(sizeof(Page), 1);
    }

  }

  DeserializeAll (???)
  char* src = (char*)page->data;
  for (int i = 0; i < rowsPerPage; i++) {
    Row* rowPtr = malloc(rowSize); // includes flexible array
    memcpy(rowPtr, src, rowSize);
    table->rows[currentRowIndex++] = rowPtr;
    src += rowSize;
}


  */


  for (int rowIndex = 0; rowIndex < table->rowCount; rowIndex++) {
    // inefficiently check page load every loop
    int pageNumber = rowIndex / table->schema.rowsPerPage;
    int rowOffset = rowIndex % table->schema.rowsPerPage;

    if (!table->pages[pageNumber]) {
      table->pages[pageNumber] = calloc(sizeof(Page), 1);
    }
    
    Row* currentRow = deserializeRowFromPage(table, table->pages[pageNumber], rowIndex);

    if (currentRow->isDeleted) continue;

    if (command->s_whereClause) {

      switch (whereClauseOperand) {
        case OP_EQ:
          char* endptr;
          int val = (int) strtol(command->s_whereClause->w_value, &endptr, 10);
          if (*endptr != '\0') {
            command->e_message = strdup("Error: invalid input: could not convert str to int.\n");
            command->type = CMD_ERROR;
            return NULL;
          }
          if (currentRow->values[whereColumnInfo->columnIndex].intValue != val) continue;
          break;

        default:
          command->e_message = strdup("Error: unrecognized operator.\n");
          command->type = CMD_ERROR;
          return NULL;
      }
    }

    if (selection->selectedRowCount >= selection->selectCapacity) {
      int oldCapacity = selection->selectCapacity;
      selection->selectCapacity *= 2;
      selection->selectedRows = realloc(selection->selectedRows, selection->selectCapacity * sizeof(SelectedRow));
    }

    selection->selectedRows[selection->selectedRowCount].values = malloc(sizeof(Value*) * command->s_colNameCount);

    // iterate usedSchemaIndex array
    // each int in the array is the position in table->rows[N]->values[] array 
    for (int usedSchemaIndex = 0; usedSchemaIndex < command->s_colNameCount; usedSchemaIndex++) {
      selection->selectedRows[selection->selectedRowCount].values[usedSchemaIndex] = &currentRow->values[selectColumnInfo[usedSchemaIndex].columnIndex];
    }
    selection->selectedRowCount++;
  }

  printSelection(table, selection, command->s_colNameCount, selectColumnInfo);

  // free(selectColumnInfo);
  if (whereColumnInfo) free(whereColumnInfo);

  if (selection->selectedRows == 0) {
    printf("No rows returned!\n");
  }

  return selection;
}

bool dropTable(Tables* tables, Command* command) {
  
  int dropTableIndex = -1;
  for (int i = 0; i < tables->tableCount; i++) {
    if (!tables->tableList[i]) continue;
    
    if (strcmp(tables->tableList[i]->name, command->tableName) == 0) {
      dropTableIndex = i;
      break;
    }
  }

  if (dropTableIndex == -1) {
    command->e_message = strdup("Error: table doesn't exist.\n");
    command->type = CMD_ERROR;
    return false;
  }

  int result = deleteTableData(tables->tableList[dropTableIndex]->name);
  command->type = CMD_ERROR;
  switch (result) {
    case 0:
      command->type = CMD_DROP;
      break;
    case -1:
      command->e_message = strdup("Error: failed to delete table metadata.\n");
      break;
    case -2:
      command->e_message = strdup("Error: failed to delete table data.\n");
      break;
    case -3:
      command->e_message = strdup("Error: failed to delete table directory.\n");
      break;
    default:
  }

  if (command->type == CMD_ERROR) return false;

  freeTable(tables->tableList[dropTableIndex]);

  for (int i = dropTableIndex; i < tables->tableCount - 1; i++) {
    tables->tableList[i] = tables->tableList[i + 1];
  }
  tables->tableCount--;
  tables->tableList[tables->tableCount] = NULL;
  return true;
}