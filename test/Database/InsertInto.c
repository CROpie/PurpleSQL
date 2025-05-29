#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "repl.h"
#include "database.h"
#include "encoder.h"
#include "testHelpers.h"

// SETUP : Manually create table
// TESTS : Write command, run InsertInto 

Tables* tables;

char* createInput;
Command* createCommand;

char* insertInput;
Command* insertCommand;

char* makeRepeatedCharString(char ch, size_t N) {
  char* result = malloc(N + 1);
  memset(result, ch, N);
  result[N] = '\0';
  return result;
}

void setUp(void) {

  tables = calloc(sizeof(Tables), 1);
  tables->tablesCapacity = TABLE_CAPACITY;
  tables->tableList = calloc(sizeof(Table*) * tables->tablesCapacity, 1);
  tables->tableCount = 0;

  char sql[] = 
  "CREATE TABLE myTable (\n"
  "id INT,\n"
  "isDeleted BOOL,\n"
  "message VARCHAR(255) );";

  createInput = strdup(sql);
  createCommand = parseInput(createInput);
  tables->tableList[tables->tableCount++] = createTable(tables, createCommand);
}

void tearDown(void) {
  freeTables(tables);
  freeCommand(createCommand);
  freeCommand(insertCommand);
  // tables = NULL;
  // createCommand = NULL;
  // insertCommand = NULL;
}

void test_insertRecord_fail_not_int(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 'error', true, 'this is my message' );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: Value not int", insertCommand->e_message);
}

void test_insertRecord_fail_not_bool(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, 'error', 'this is my message' );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: Value not bool", insertCommand->e_message);
}

void test_insertRecord_fail_not_enough_values(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  // this error is caught by parseInput
  // insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: Mismatch between number of columns and values", insertCommand->e_message);
}

void test_insertRecord_fail_too_many_values(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message', 'error' );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  // this error is caught by parseInput
  // insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: Mismatch between number of columns and values", insertCommand->e_message);
}

void test_insertRecord_fail_col_not_in_schema(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, text )"
  "VALUES ( 1, true, 'this is my message' );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: unrecognized column entered", insertCommand->e_message);
}

void test_insertRecord_fail_too_many_cols(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message, oneMore )"
  "VALUES ( 1, true, 'this is my message', 47 );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: unrecognized column entered", insertCommand->e_message);
}

void test_insertRecord_fail_unknown_table(void) {
  
  char insertSql[] = 
  "INSERT INTO yourTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message' );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: table not found", insertCommand->e_message);
}

void test_insertRecord_fail_duplicate_column(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, id )"
  "VALUES ( 1, true, 23 );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: duplicate column detected", insertCommand->e_message);
}

void test_insertRecord_fail_long_string(void) {

  char* longStr = makeRepeatedCharString('A', VARCHAR_LENGTH);
  char insertSql[1024];

  snprintf(insertSql, sizeof(insertSql), 
    "INSERT INTO myTable ( id, isDeleted, message )"
    "VALUES ( 1, true, '%s' );", longStr);  

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, insertCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: String is too long", insertCommand->e_message);
}

void test_insertRecord_success_string_is_on_limit(void) {

  char* longStr = makeRepeatedCharString('A', VARCHAR_LENGTH) - 1;
  char insertSql[1024];

  snprintf(insertSql, sizeof(insertSql), 
    "INSERT INTO myTable ( id, isDeleted, message )"
    "VALUES ( 1, true, '%s' );", longStr);  

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  TEST_ASSERT_EQUAL(CMD_INSERT, insertCommand->type);
}

void test_insertRecord_success_2nd_page(void) {

  int ROW_NUM = 25;
  int MAX_LEN = 4096;
  char insertSql[MAX_LEN];
  int offset = 0;

  offset += snprintf(insertSql + offset, MAX_LEN - offset,
    "INSERT INTO myTable ( id, isDeleted, message) VALUES ");
  
  for (int i = 0; i < ROW_NUM; i++) {
    offset += snprintf(insertSql + offset, MAX_LEN - offset, 
      "( %d, %s, '%s')%s ",
      i, "true", "this is my message",
      (i == ROW_NUM - 1) ? ";" : ",");
  }

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);

  Row* expectedRow_0 = calloc(sizeof(Row) + sizeof(Value) * 3, 1);
  expectedRow_0->isDeleted = false;
  expectedRow_0->values[0].intValue = 0;
  expectedRow_0->values[1].boolValue = true;
  strcpy(expectedRow_0->values[2].stringValue, "this is my message");

  Row* actualRow_0 = deserializeRowFromPage(tables->tableList[0], tables->tableList[0]->pages[0], 0);

  Row* expectedRow_1 = calloc(sizeof(Row) + sizeof(Value) * 3, 1);
  expectedRow_1->isDeleted = false;
  expectedRow_1->values[0].intValue = ROW_NUM - 1;
  expectedRow_1->values[1].boolValue = true;
  strcpy(expectedRow_1->values[2].stringValue, "this is my message");

  // at the end of insertion, rowCount should be ROW_NUM
  TEST_ASSERT_EQUAL(ROW_NUM, tables->tableList[0]->rowCount);

  int pageNumber = tables->tableList[0]->rowCount / tables->tableList[0]->schema.rowsPerPage;

  // expecting a second page after this many rows inserted
  TEST_ASSERT_EQUAL(1, pageNumber);


  // rowOffset is 5 with tables->tableList[0]->rowCount. That is the next position to add to
  int rowOffset = (tables->tableList[0]->rowCount - 1) % tables->tableList[0]->schema.rowsPerPage;

  // -> rowOffset is 4

  // rowsPerPage is 20
  // TEST_ASSERT_EQUAL(1, tables->tableList[0]->schema.rowsPerPage);

  Row* actualRow_1 = deserializeRowFromPage(tables->tableList[0], tables->tableList[0]->pages[pageNumber], rowOffset);

  TEST_ASSERT_EQUAL(expectedRow_0->values[0].intValue, actualRow_0->values[0].intValue);
  TEST_ASSERT_EQUAL(expectedRow_1->values[0].intValue, actualRow_1->values[0].intValue);

  free(expectedRow_0);
  free(actualRow_0);
  free(expectedRow_1);
  free(actualRow_1);
}

void test_insertRecord_success_3cols_1row(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message' );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

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

  free(expectedRow);
  free(actualRow);
}

void test_insertRecord_success_3cols_1row_twice(void) {

  char insertSqlOne[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message' );";

  insertInput = strdup(insertSqlOne);
  insertCommand = parseInput(insertInput);

  insertRecord(tables, insertCommand);
  
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

  freeCommand(insertCommandTwo);
}

void test_insertRecord_success_3cols_2rows(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES"
  "( 1, true, 'this is my message' ),"
  "( 2, false, 'my second message' );";

  insertInput = strdup(insertSql);
  insertCommand = parseInput(insertInput);

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

  TEST_ASSERT_EQUAL(CMD_INSERT, insertCommand->type);

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
}

int main(void) {
    UNITY_BEGIN();

    // happy
    RUN_TEST(test_insertRecord_success_3cols_1row);
    RUN_TEST(test_insertRecord_success_3cols_1row_twice);
    RUN_TEST(test_insertRecord_success_3cols_2rows);
    RUN_TEST(test_insertRecord_success_string_is_on_limit);
    RUN_TEST(test_insertRecord_success_2nd_page);

    // unhappy
    RUN_TEST(test_insertRecord_fail_not_int);
    RUN_TEST(test_insertRecord_fail_not_bool);
    RUN_TEST(test_insertRecord_fail_not_enough_values);
    RUN_TEST(test_insertRecord_fail_too_many_values);
    RUN_TEST(test_insertRecord_fail_col_not_in_schema);
    RUN_TEST(test_insertRecord_fail_too_many_cols);
    RUN_TEST(test_insertRecord_fail_duplicate_column);
    RUN_TEST(test_insertRecord_fail_unknown_table);
    RUN_TEST(test_insertRecord_fail_long_string);

    return UNITY_END();
}
