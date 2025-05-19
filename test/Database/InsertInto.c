#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "repl.h"
#include "database.h"

Table* table;

void setUp(void) {

  table = calloc(sizeof(Table), 1);
  table->schema.columnCount = 3;
  table->rowCapacity = ROW_CAPACITY;

  table->rows = malloc(sizeof(Row) * table->rowCapacity);

  table->schema.columns = malloc(sizeof(ColumnSchema) * table->schema.columnCount);

  strcpy(table->schema.columns[0].name, "id");
  table->schema.columns[0].type = COL_INT;

  strcpy(table->schema.columns[1].name, "isDeleted");
  table->schema.columns[1].type = COL_BOOL;

  strcpy(table->schema.columns[2].name, "message");
  table->schema.columns[2].type = COL_STRING;
}

void tearDown(void) {
  freeTable(table);
}

/* INSERT RECORD */

void test_insertRecord_success_3cols_1row(void) {
  
  char insertSql[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message' );";

  char* insertInput = strdup(insertSql);
  Command* insertCommand = parseInput(insertInput);

  insertRecord(table, insertCommand);

  TEST_ASSERT_EQUAL(table->rows[0]->values[0].intValue, 1);
  TEST_ASSERT_EQUAL(table->rows[0]->values[1].boolValue, true);
  TEST_ASSERT_EQUAL_STRING(table->rows[0]->values[2].stringValue, "this is my message");

  freeCommand(insertCommand);
}

void test_insertRecord_success_3cols_1row_twice(void) {

  char insertSqlOne[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 1, true, 'this is my message' );";

  char* insertInputOne = strdup(insertSqlOne);
  Command* insertCommandOne = parseInput(insertInputOne);

  insertRecord(table, insertCommandOne);
  
  char insertSqlTwo[] = 
  "INSERT INTO myTable ( id, isDeleted, message )"
  "VALUES ( 2, false, 'another message' );";

  char* insertInputTwo = strdup(insertSqlTwo);
  Command* insertCommandTwo = parseInput(insertInputTwo);

  insertRecord(table, insertCommandTwo);

  TEST_ASSERT_EQUAL(table->rows[0]->values[0].intValue, 1);
  TEST_ASSERT_EQUAL(table->rows[0]->values[1].boolValue, true);
  TEST_ASSERT_EQUAL_STRING(table->rows[0]->values[2].stringValue, "this is my message");

  TEST_ASSERT_EQUAL(table->rows[1]->values[0].intValue, 2);
  TEST_ASSERT_EQUAL(table->rows[1]->values[1].boolValue, false);
  TEST_ASSERT_EQUAL_STRING(table->rows[1]->values[2].stringValue, "another message");

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

  insertRecord(table, insertCommand);

  TEST_ASSERT_EQUAL(table->rows[0]->values[0].intValue, 1);
  TEST_ASSERT_EQUAL(table->rows[0]->values[1].boolValue, true);
  TEST_ASSERT_EQUAL_STRING(table->rows[0]->values[2].stringValue, "this is my message");

  TEST_ASSERT_EQUAL(table->rows[1]->values[0].intValue, 2);
  TEST_ASSERT_EQUAL(table->rows[1]->values[1].boolValue, false);
  TEST_ASSERT_EQUAL_STRING(table->rows[1]->values[2].stringValue, "my second message");

  freeCommand(insertCommand);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_insertRecord_success_3cols_1row);
    RUN_TEST(test_insertRecord_success_3cols_1row_twice);
    RUN_TEST(test_insertRecord_success_3cols_2rows);

    return UNITY_END();
}
