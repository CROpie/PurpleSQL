#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "repl.h"
#include "database.h"

void setUp(void) {
  //
}

void tearDown(void) {
}

void test_createTable_fail_no_tableName(void) {
  Command* command = calloc(1, sizeof(Command));
  Table* table = createTable(command);

  TEST_ASSERT_NULL(table);

  freeCommand(command);
  freeTable(table);
}

void test_createTable_success_3cols(void) {
  
  char* tableName = "myTable";

  // Doesn't work with freeCommand(command) because these pairs are in the stack  
  // ColPair columnPairs[] = {
  //   { "id", "INT" },
  //   { "isDeleted", "BOOL" },
  //   { "message", "VARCHAR(255)" }
  // };

  Command* command = calloc(1, sizeof(Command));
  command->tableName = malloc(strlen(tableName) + 1);
  strcpy(command->tableName, tableName);
  
  command->c_numColPairs = 3;

  command->c_colPairs = malloc(sizeof(ColPair) * command->c_numColPairs);

  command->c_colPairs[0].colName = strdup("id");
  command->c_colPairs[0].colDef  = strdup("INT");

  command->c_colPairs[1].colName = strdup("isDeleted");
  command->c_colPairs[1].colDef  = strdup("BOOL");

  command->c_colPairs[2].colName = strdup("message");
  command->c_colPairs[2].colDef  = strdup("VARCHAR(255)");

  Table* table = createTable(command);

  TEST_ASSERT_NOT_NULL(table);
  TEST_ASSERT_EQUAL(3, table->schema.columnCount);

  TEST_ASSERT_EQUAL_STRING("id", table->schema.columns[0].name);
  TEST_ASSERT_EQUAL(COL_INT, table->schema.columns[0].type);

  TEST_ASSERT_EQUAL_STRING("isDeleted", table->schema.columns[1].name);
  TEST_ASSERT_EQUAL(COL_BOOL, table->schema.columns[1].type);

  TEST_ASSERT_EQUAL_STRING("message", table->schema.columns[2].name);
  TEST_ASSERT_EQUAL(COL_STRING, table->schema.columns[2].type);

  TEST_ASSERT_EQUAL(0, table->rowCount);

  // Clean up after the test
  freeCommand(command);
  freeTable(table);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_createTable_success_3cols);
    RUN_TEST(test_createTable_fail_no_tableName);

    return UNITY_END();
}
