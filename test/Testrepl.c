#include <stdio.h>
#include <stdlib.h>

#include "unity.h"
#include "database.h"
#include "repl.h"

void setUp(void) {
  //
}

void tearDown(void) {
}

// Test the parseInput->CREATE TABLE function
void test_parseInput_CreateTable_3cols(void) {
  
  char sql[] = 
    "CREATE TABLE myTable (\n"
    "id INT,\n"
    "isDeleted BOOL,\n"
    "message VARCHAR(255) );";

  Command* command = parseInput(sql);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_CREATE, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->columnCount);

  TEST_ASSERT_EQUAL_STRING("id", command->columnNames[0]);
  TEST_ASSERT_EQUAL(COL_INT, command->columnTypes[0]);

  TEST_ASSERT_EQUAL_STRING("isDeleted", command->columnNames[1]);
  TEST_ASSERT_EQUAL(COL_BOOL, command->columnTypes[1]);

  TEST_ASSERT_EQUAL_STRING("message", command->columnNames[2]);
  TEST_ASSERT_EQUAL(COL_STRING, command->columnTypes[2]);

  // Clean up after the test
  for (int i = 0; i < command->columnCount; i++) {
    free(command->columnNames[i]);
  }

  free(command->columnNames);
  free(command->columnTypes);
  free(command);
}

void test_parseInput_CreateTable_noTableName(void) {
  
  char sql[] = 
    "CREATE TABLE (\n"
    "id INT,\n"
    "isDeleted BOOL,\n"
    "message VARCHAR(255) );";

  Command* command = parseInput(sql);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_ERROR, command->type);

  free(command);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_parseInput_CreateTable_3cols);

  RUN_TEST(test_parseInput_CreateTable_noTableName);

  return UNITY_END();
}
