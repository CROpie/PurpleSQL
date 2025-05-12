#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"
#include "repl.h"



/* HELPER FUNCTIONS */
// Add a string to an array, re-allocating if necessary
void addStrToArray(char*** array, int* size, int* capacity, const char* value) {
  if (*size >= *capacity) {
    *capacity *= 2;
    *array = realloc(*array, (*capacity) * sizeof(char*));

    if (!*array) {
      printf("realloc failed.\n");
      exit(EXIT_FAILURE);
    }
  }
  // (*array)[(*size)++] = strdup(value);
  size_t len = strlen(value) + 1;
  char* copy = calloc(len, 1);
  strcpy(copy, value);
  (*array)[(*size)++] = copy;
}

// print a string and its length
void prStr(char* str) {
  printf("str %s length %ld\n", str, strlen(str));
}

// remove leading and trailing whitespace
void trim(char* str) {
  char* start = str;

  while (*start == ' ') start++;

  if (start != str) {
    memmove(str, start, strlen(start) + 1);
  }

  char* end = str + strlen(str) - 1;
  while (end > str && (*end == ' ' || *end == '\n')) {
    *end = '\0';
    end--;
  }
}

// convert string to column type
ColumnType convertStrToColumnType(char* columnType) {
  ColumnType type = COL_UNDEFINED;

  if (strncmp(columnType, "INT", 3) == 0) {
    type = COL_INT;
  }

  if (strncmp(columnType, "BOOL", 4) == 0) {
    type = COL_BOOL;
  }

  if (strncmp(columnType, "VARCHAR(255)", 12) == 0) {
    type = COL_STRING;
  }

  return type;
}

void parseCreate(char* input, Command* command) {

  char* lessCommand = input + strlen("CREATE TABLE ");
  trim(lessCommand);

  char* tableName = strtok(lessCommand, "(");
  if (!tableName) {
    printf("Empty table name\n");
    command->type = CMD_ERROR;
    return;
  }

  trim(tableName);

  command->tableName = malloc(strlen(tableName) + 1);
  strcpy(command->tableName, tableName);

  char** colNames = malloc(sizeof(char*) * COL_CAPACITY);
  int colNamesLength = 0;
  int colNamesCapacity = COL_CAPACITY;

  char** colTypes = malloc(sizeof(char*) * COL_CAPACITY);
  int colTypesLength = 0;
  int colTypesCapacity = COL_CAPACITY;

  while (true) {

    char* colName = strtok(NULL, " \n");
    if (!colName) break;

    trim(colName);

    addStrToArray(&colNames, &colNamesLength, &colNamesCapacity, colName);

    char* colType = strtok(NULL, " \n");
    if (!colType) break;

    trim(colType);

    char* end = colType + strlen(colType) - 1;

    bool endsWithComma = (*end == ',');

    if (endsWithComma) *end = '\0'; 

    addStrToArray(&colTypes, &colTypesLength, &colTypesCapacity, colType);

    if (!endsWithComma) break;
  }

  if (colNamesLength != colTypesLength) {
    printf("Column names and types mismatch!\n");
    command->type = CMD_ERROR;
    return;
  }

  command->columnCount = colNamesLength;
  command->columnNames = colNames;

  command->columnTypes = malloc(sizeof(ColumnType) * colTypesLength);

  for (int i = 0; i < colTypesLength; i++) {
    command->columnTypes[i] = convertStrToColumnType(colTypes[i]);
    if (command->columnTypes[i] == COL_UNDEFINED) command->type = CMD_ERROR;
    free(colTypes[i]);
  }
  free(colTypes);

}

char* getInput() {
  char buffer[128];
  char* input = calloc(INPUT_LENGTH, 1);
  if (!input) return NULL;

  while (fgets(buffer, sizeof(buffer), stdin)) {
    strcat(input, buffer);
    if (strchr(buffer, ';')) {
      break;
    }
    printf("  -> ");
  }

  // remove all newlines
  for (int i = 0; input[i]; i++) {
    if (input[i] == '\n') input[i] == ' ';
  }

  // validation to ensure use input was not too long

  return input;
}

Command* parseInput(char* input) {
  Command* command = calloc(sizeof(Command), 1);
  command->type = CMD_UNDEFINED;

  if (strncmp(input, "CREATE TABLE ", 13) == 0) {
    command->type = CMD_CREATE;
    parseCreate(input, command);
  }

  if (strncmp(input, "exit", 4) == 0) {
    command->type = CMD_EXIT;
  }

  free(input);

  return command;
}
