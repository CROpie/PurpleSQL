#include <stdio.h>
#include <stdlib.h>

#include "unity.h"
#include "repl.h"
#include "database.h"




void setUp(void) {
  //
}

void tearDown(void) {
}

/* INSERT RECORD */

void test_insertRecord_success_3cols(void) {
  
  char createSql[] = 
    "CREATE TABLE myTable (\n"
    "id INT,\n"
    "isDeleted BOOL,\n"
    "message VARCHAR(255) );";

  Command* createCommand = parseInput(createSql);

  Table* table = createTable(createCommand);

    char insertSql[] = 
    "INSERT INTO myTable (id, isDeleted, message )"
    "VALUES (1, true, 'this is my message' ), (2, false, 'actually this is');";

  Command* insertCommand = parseInput(insertSql);

  insertRecord(table, insertCommand);

  char selectSql[] = 
    "SELECT * FROM myTable;";

  Command* selectCommand = parseInput(selectSql);

  selectColumns(table, selectCommand);
}

/* CREATE TABLE */

void test_createTable_fail_no_tableName(void) {
  Command* command = calloc(1, sizeof(Command));
  Table* table = createTable(command);

  TEST_ASSERT_NULL(table);

  free(command);
}

void test_createTable_success_3cols(void) {
  
  char* tableName = "myTable";
  
  ColPair columnPairs[] = {
    { "id", "INT" },
    { "isDeleted", "BOOL" },
    { "message", "VARCHAR(255)" }
  };

  Command* command = calloc(1, sizeof(Command));
  command->tableName = tableName;
  command->c_numColPairs = 3;
  command->c_colPairs = columnPairs;

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

    // RUN_TEST(test_createTable_success_3cols);
    // RUN_TEST(test_createTable_fail_no_tableName);
    RUN_TEST(test_insertRecord_success_3cols);

    return UNITY_END();
}
