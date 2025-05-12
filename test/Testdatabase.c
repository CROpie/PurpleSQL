#include <stdio.h>
#include <stdlib.h>

#include "unity.h"
#include "database.h"

void setUp(void) {
  //
}

void tearDown(void) {
}


/* CREATE TABLE */

void test_createTable_fail_no_tableName(void) {
  Command* command = calloc(1, sizeof(Command));
  Table* table = createTable(command);

  TEST_ASSERT_NULL(table);

  free(command);
}


// Test the createTable function
void test_createTable_success_3cols(void) {
  
  char* tableName = "myTable";
  
  char* columnNames[3] = {
      "id",
      "isDeleted",
      "message"
  };
  
  ColumnType columnTypes[3] = {
      COL_INT,
      COL_BOOL,
      COL_STRING
  };

  Command* command = calloc(1, sizeof(Command));
  command->tableName = tableName;
  command->columnCount = 3;
  command->columnNames = columnNames;
  command->columnTypes = columnTypes;

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
  TEST_ASSERT_NULL(table->rows);

  // Clean up after the test
  free(command);
  free(table->schema.columns);
  free(table);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_createTable_success_3cols);
    RUN_TEST(test_createTable_fail_no_tableName);

    return UNITY_END();
}
