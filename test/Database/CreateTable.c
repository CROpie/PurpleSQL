#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "repl.h"
#include "database.h"
#include "testHelpers.h"

Table* table;
Command* command;

// SETUP : 
// TESTS : Write Command, run CreateTable

void setUp(void) {
  //
}

void tearDown(void) {
  freeCommand(command);
  freeTable(table);
}

void test_createTable_fail_no_tableName(void) {
  command = calloc(1, sizeof(Command));

  command->c_numColPairs = 3;

  command->c_colPairs = calloc(sizeof(ColPair) * command->c_numColPairs, 1);

  command->c_colPairs[0].colName = strdup("id");
  command->c_colPairs[0].colDef  = strdup("INT");

  command->c_colPairs[1].colName = strdup("isDeleted");
  command->c_colPairs[1].colDef  = strdup("BOOL");

  command->c_colPairs[2].colName = strdup("message");
  command->c_colPairs[2].colDef  = strdup("VARCHAR(255)");

  table = createTable(command);

  TEST_ASSERT_NULL(table);

  TEST_ASSERT_STRING_CONTAINS("Error: No table name", command->e_message);
}

void test_createTable_fail_no_columns(void) {
  command = calloc(1, sizeof(Command));

  char* tableName = "myTable";
  command->tableName = malloc(strlen(tableName) + 1);
  strcpy(command->tableName, tableName);

  command->c_numColPairs = 0;

  table = createTable(command);

  TEST_ASSERT_NULL(table);

  TEST_ASSERT_STRING_CONTAINS("Error: No columns", command->e_message);
  TEST_ASSERT_STRING_CONTAINS("Error: No column definitions", command->e_message);
}

void test_createTable_fail_unknown_column_type(void) {
  command = calloc(1, sizeof(Command));

  char* tableName = "myTable";
  command->tableName = malloc(strlen(tableName) + 1);
  strcpy(command->tableName, tableName);

  command->c_numColPairs = 3;

  command->c_colPairs = calloc(sizeof(ColPair) * command->c_numColPairs, 1);

  command->c_colPairs[0].colName = strdup("id");
  command->c_colPairs[0].colDef  = strdup("NUMBER");

  command->c_colPairs[1].colName = strdup("isDeleted");
  command->c_colPairs[1].colDef  = strdup("BOOL");

  command->c_colPairs[2].colName = strdup("message");
  command->c_colPairs[2].colDef  = strdup("VARCHAR(255)");

  table = createTable(command);

  TEST_ASSERT_NULL(table);

  TEST_ASSERT_STRING_CONTAINS("Error: Unknown column type", command->e_message);
}

void test_createTable_fail_too_many_columns(void) {
  
  command = calloc(1, sizeof(Command));

  char* tableName = "myTable";
  command->tableName = malloc(strlen(tableName) + 1);
  strcpy(command->tableName, tableName);
  
  command->c_numColPairs = 100;

  command->c_colPairs = calloc(sizeof(ColPair) * command->c_numColPairs, 1);

  for (int i = 0; i < command->c_numColPairs; i++) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "int_%d", i);
    command->c_colPairs[i].colName = strdup(buffer);
    command->c_colPairs[i].colDef  = strdup("INT"); 
  }

  table = createTable(command);

  TEST_ASSERT_NULL(table);

  TEST_ASSERT_STRING_CONTAINS("Error: Too many columns", command->e_message);
}

void test_createTable_success_3cols(void) {
  
  command = calloc(1, sizeof(Command));

  char* tableName = "myTable";
  command->tableName = malloc(strlen(tableName) + 1);
  strcpy(command->tableName, tableName);
  
  command->c_numColPairs = 3;

  command->c_colPairs = calloc(sizeof(ColPair) * command->c_numColPairs, 1);

  command->c_colPairs[0].colName = strdup("id");
  command->c_colPairs[0].colDef  = strdup("INT");

  command->c_colPairs[1].colName = strdup("isDeleted");
  command->c_colPairs[1].colDef  = strdup("BOOL");

  command->c_colPairs[2].colName = strdup("message");
  command->c_colPairs[2].colDef  = strdup("VARCHAR(255)");

  table = createTable(command);

  TEST_ASSERT_NOT_NULL(table);
  TEST_ASSERT_EQUAL_STRING("myTable", table->name);

  TEST_ASSERT_EQUAL(3, table->schema.columnCount);

  TEST_ASSERT_EQUAL_STRING("id", table->schema.columns[0].name);
  TEST_ASSERT_EQUAL(COL_INT, table->schema.columns[0].type);

  TEST_ASSERT_EQUAL_STRING("isDeleted", table->schema.columns[1].name);
  TEST_ASSERT_EQUAL(COL_BOOL, table->schema.columns[1].type);

  TEST_ASSERT_EQUAL_STRING("message", table->schema.columns[2].name);
  TEST_ASSERT_EQUAL(COL_STRING, table->schema.columns[2].type);

  TEST_ASSERT_EQUAL(0, table->rowCount);

  TEST_ASSERT_EQUAL(0, table->pageCount);
  TEST_ASSERT_EQUAL(PAGE_CAPACITY, table->pageCapacity);

  size_t rowSize = sizeof(Row) + sizeof(Value) * table->schema.columnCount;
  int rowsPerPage = PAGE_SIZE / rowSize;
  TEST_ASSERT_EQUAL(rowsPerPage, table->schema.rowsPerPage);

}

void test_createTable_success_12cols(void) {
  
  command = calloc(1, sizeof(Command));

  char* tableName = "myTable";
  command->tableName = malloc(strlen(tableName) + 1);
  strcpy(command->tableName, tableName);
  
  command->c_numColPairs = 12;

  command->c_colPairs = calloc(sizeof(ColPair) * command->c_numColPairs, 1);

  for (int i = 0; i < 12; i++) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "int_%d", i);
    command->c_colPairs[i].colName = strdup(buffer);
    command->c_colPairs[i].colDef  = strdup("INT"); 
  }

  table = createTable(command);

  TEST_ASSERT_NOT_NULL(table);
  TEST_ASSERT_EQUAL_STRING("myTable", table->name);

  TEST_ASSERT_EQUAL(12, table->schema.columnCount);

  TEST_ASSERT_EQUAL_STRING("int_0", table->schema.columns[0].name);
  TEST_ASSERT_EQUAL(COL_INT, table->schema.columns[0].type);

  TEST_ASSERT_EQUAL_STRING("int_11", table->schema.columns[11].name);
  TEST_ASSERT_EQUAL(COL_INT, table->schema.columns[11].type);

  TEST_ASSERT_EQUAL(0, table->rowCount);

  TEST_ASSERT_EQUAL(0, table->pageCount);
  TEST_ASSERT_EQUAL(PAGE_CAPACITY, table->pageCapacity);

  size_t rowSize = sizeof(Row) + sizeof(Value) * table->schema.columnCount;
  int rowsPerPage = PAGE_SIZE / rowSize;
  TEST_ASSERT_EQUAL(rowsPerPage, table->schema.rowsPerPage);
}

int main(void) {
    UNITY_BEGIN();

    // happy
    RUN_TEST(test_createTable_success_3cols);
    RUN_TEST(test_createTable_success_12cols);

    // unhappy
    RUN_TEST(test_createTable_fail_no_tableName);
    RUN_TEST(test_createTable_fail_no_columns);
    RUN_TEST(test_createTable_fail_unknown_column_type);
    RUN_TEST(test_createTable_fail_too_many_columns);

    return UNITY_END();
}
