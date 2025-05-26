#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "repl.h"
#include "database.h"

// SETUP : Manually create table, insert 2 rows
// TESTS : Write command, run SelectWhere

Tables* tables;

void setUp(void) {

  /* INITIALIZE TABLES ARRAY */
  tables = calloc(sizeof(Tables), 1);
  tables->tableList = calloc(sizeof(Table*) * tables->tablesCapacity, 1);
  tables->tableCount = 0;

  /* CREATE TABLE */
  char createSql[] = 
  "CREATE TABLE myTable (\n"
  "id INT,\n"
  "isDeleted BOOL,\n"
  "message VARCHAR(255) );";

  char* createInput = strdup(createSql);
  Command* createCommand = parseInput(createInput);
  tables->tableList[tables->tableCount++] = createTable(createCommand);

  /* INSERT INTO */
  char insertSql[] = 
  "INSERT INTO myTable (id, isDeleted, message)"
  "VALUES (1, false, 'first message'), (2, true, 'second message');";

  char* insertInput = strdup(insertSql);
  Command* insertCommand = parseInput(insertInput);
  insertRecord(tables, insertCommand);
}

void tearDown(void) {
  freeTables(tables);
}


// 
void test_selectWhere_success_star(void) {
  
  char selectSql[] = 
  "SELECT * FROM myTable;";

  char* selectInput = strdup(selectSql);
  Command* selectCommand = parseInput(selectInput);

  Selection* selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 2);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 1);
  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[1]->boolValue, false);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[0].values[2]->stringValue, "first message");

  TEST_ASSERT_EQUAL(selection->selectedRows[1].values[0]->intValue, 2);
  TEST_ASSERT_EQUAL(selection->selectedRows[1].values[1]->boolValue, true);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[1].values[2]->stringValue, "second message");

  freeSelection(selection);
  freeCommand(selectCommand);
}

void test_selectWhere_success_id_only(void) {
  
  char selectSql[] = 
  "SELECT id FROM myTable;";

  char* selectInput = strdup(selectSql);
  Command* selectCommand = parseInput(selectInput);

  Selection* selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 2);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 1);

  freeSelection(selection);
  freeCommand(selectCommand);
}

void test_selectWhere_success_id_and_message(void) {
  
  char selectSql[] = 
  "SELECT id, message FROM myTable;";

  char* selectInput = strdup(selectSql);
  Command* selectCommand = parseInput(selectInput);

  Selection* selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 2);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 1);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[0].values[1]->stringValue, "first message");

  TEST_ASSERT_EQUAL(selection->selectedRows[1].values[0]->intValue, 2);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[1].values[1]->stringValue, "second message");

  freeSelection(selection);
  freeCommand(selectCommand);
}

void test_selectWhere_success_id_only_where_one_id(void) {
  
  char selectSql[] = 
  "SELECT id FROM myTable WHERE id = 2;";

  char* selectInput = strdup(selectSql);
  Command* selectCommand = parseInput(selectInput);

  Selection* selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 1);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 2);
  TEST_ASSERT_NULL(selection->selectedRows[1].values);

  freeSelection(selection);
  freeCommand(selectCommand);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_selectWhere_success_star);
    RUN_TEST(test_selectWhere_success_id_only);
    RUN_TEST(test_selectWhere_success_id_and_message);
    RUN_TEST(test_selectWhere_success_id_only_where_one_id);

    return UNITY_END();
}