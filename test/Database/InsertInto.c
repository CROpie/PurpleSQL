#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "repl.h"
#include "database.h"
#include "encoder.h"

// SETUP : Manually create table
// TESTS : Write command, run InsertInto 

Tables* tables;

void setUp(void) {

  tables = calloc(sizeof(Tables), 1);
  tables->tableList = calloc(sizeof(Table*) * tables->tablesCapacity, 1);
  tables->tableCount = 0;

  char sql[] = 
  "CREATE TABLE myTable (\n"
  "id INT,\n"
  "isDeleted BOOL,\n"
  "message VARCHAR(255) );";

  char* input = strdup(sql);
  Command* command = parseInput(input);
  tables->tableList[tables->tableCount++] = createTable(command);
}

void tearDown(void) {
  freeTables(tables);
}


void test_insertRecord_success_3cols_1row(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message' );";

  char* insertInput = strdup(insertSql);
  Command* insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  Row* expectedRow = calloc(sizeof(Row) + sizeof(Value) * 3, 1);
  expectedRow->isDeleted = false;
  expectedRow->values[0].intValue = 1;
  expectedRow->values[1].boolValue = true;
  strcpy(expectedRow->values[2].stringValue, "this is my message");

  Row* actualRow = deserializeRowFromPage(tables->tableList[0], tables->tableList[0]->pages[0], 0);

  TEST_ASSERT_EQUAL(actualRow->isDeleted, expectedRow->isDeleted);
  TEST_ASSERT_EQUAL(actualRow->values[0].intValue, expectedRow->values[0].intValue);
  TEST_ASSERT_EQUAL(actualRow->values[1].boolValue, expectedRow->values[1].boolValue);
  TEST_ASSERT_EQUAL_STRING(actualRow->values[2].stringValue, expectedRow->values[2].stringValue);

  // free(expectedRow);
  // free(actualRow);

  freeCommand(insertCommand);

}

void test_insertRecord_success_3cols_1row_twice(void) {

  char insertSqlOne[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message' );";

  char* insertInputOne = strdup(insertSqlOne);
  Command* insertCommandOne = parseInput(insertInputOne);

  insertRecord(tables, insertCommandOne);
  
  char insertSqlTwo[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 2, false, 'another message' );";

  char* insertInputTwo = strdup(insertSqlTwo);
  Command* insertCommandTwo = parseInput(insertInputTwo);

  insertRecord(tables, insertCommandTwo);

  Row* expectedRow_0 = calloc(sizeof(Row) + sizeof(Value) * 3, 1);
  expectedRow_0->isDeleted = false;
  expectedRow_0->values[0].intValue = 1;
  expectedRow_0->values[1].boolValue = true;
  strcpy(expectedRow_0->values[2].stringValue, "this is my message");
  int rowOffset_0 = 0;

  Row* expectedRow_1 = calloc(sizeof(Row) + sizeof(Value) * 3, 1);
  expectedRow_1->isDeleted = false;
  expectedRow_1->values[0].intValue = 2;
  expectedRow_1->values[1].boolValue = false;
  strcpy(expectedRow_1->values[2].stringValue, "another message");
  int rowOffset_1 = 1 % 3;

  Row* actualRow_0 = deserializeRowFromPage(tables->tableList[0], tables->tableList[0]->pages[0], rowOffset_0);
  Row* actualRow_1 = deserializeRowFromPage(tables->tableList[0], tables->tableList[0]->pages[0], rowOffset_1);

  TEST_ASSERT_EQUAL(actualRow_0->isDeleted, expectedRow_0->isDeleted);
  TEST_ASSERT_EQUAL(actualRow_0->values[0].intValue, expectedRow_0->values[0].intValue);
  TEST_ASSERT_EQUAL(actualRow_0->values[1].boolValue, expectedRow_0->values[1].boolValue);
  TEST_ASSERT_EQUAL_STRING(actualRow_0->values[2].stringValue, expectedRow_0->values[2].stringValue);

  TEST_ASSERT_EQUAL(actualRow_1->isDeleted, expectedRow_1->isDeleted);
  TEST_ASSERT_EQUAL(actualRow_1->values[0].intValue, expectedRow_1->values[0].intValue);
  TEST_ASSERT_EQUAL(actualRow_1->values[1].boolValue, expectedRow_1->values[1].boolValue);
  TEST_ASSERT_EQUAL_STRING(actualRow_1->values[2].stringValue, expectedRow_1->values[2].stringValue);

  free(expectedRow_0);
  free(expectedRow_1);
  free(actualRow_0);
  free(actualRow_1);

  freeCommand(insertCommandOne);
  freeCommand(insertCommandTwo);
}

void test_insertRecord_success_3cols_2rows(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES"
  "( 1, true, 'this is my message' ),"
  "( 2, false, 'my second message' );";

  char* insertInput = strdup(insertSql);
  Command* insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  Row* expectedRow_0 = calloc(sizeof(Row) + sizeof(Value) * 3, 1);
  expectedRow_0->isDeleted = false;
  expectedRow_0->values[0].intValue = 1;
  expectedRow_0->values[1].boolValue = true;
  strcpy(expectedRow_0->values[2].stringValue, "this is my message");
  int rowOffset_0 = 0;

  Row* expectedRow_1 = calloc(sizeof(Row) + sizeof(Value) * 3, 1);
  expectedRow_1->isDeleted = false;
  expectedRow_1->values[0].intValue = 2;
  expectedRow_1->values[1].boolValue = false;
  strcpy(expectedRow_1->values[2].stringValue, "my second message");
  int rowOffset_1 = 1 % 3;

  Row* actualRow_0 = deserializeRowFromPage(tables->tableList[0], tables->tableList[0]->pages[0], rowOffset_0);
  Row* actualRow_1 = deserializeRowFromPage(tables->tableList[0], tables->tableList[0]->pages[0], rowOffset_1);

  TEST_ASSERT_EQUAL(actualRow_0->isDeleted, expectedRow_0->isDeleted);
  TEST_ASSERT_EQUAL(actualRow_0->values[0].intValue, expectedRow_0->values[0].intValue);
  TEST_ASSERT_EQUAL(actualRow_0->values[1].boolValue, expectedRow_0->values[1].boolValue);
  TEST_ASSERT_EQUAL_STRING(actualRow_0->values[2].stringValue, expectedRow_0->values[2].stringValue);

  TEST_ASSERT_EQUAL(actualRow_1->isDeleted, expectedRow_1->isDeleted);
  TEST_ASSERT_EQUAL(actualRow_1->values[0].intValue, expectedRow_1->values[0].intValue);
  TEST_ASSERT_EQUAL(actualRow_1->values[1].boolValue, expectedRow_1->values[1].boolValue);
  TEST_ASSERT_EQUAL_STRING(actualRow_1->values[2].stringValue, expectedRow_1->values[2].stringValue);

  free(expectedRow_0);
  free(expectedRow_1);
  free(actualRow_0);
  free(actualRow_1);

  freeCommand(insertCommand);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_insertRecord_success_3cols_1row);
    RUN_TEST(test_insertRecord_success_3cols_1row_twice);
    RUN_TEST(test_insertRecord_success_3cols_2rows);

    return UNITY_END();
}
