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

  tables = malloc(sizeof(Tables));
  tables->tableList = malloc(sizeof(Table*) * tables->tablesCapacity);
  tables->tableCount = 0;

  Table* table = calloc(sizeof(Table), 1);

  table->name = malloc(strlen("myTable") + 1);
  strcpy(table->name, "myTable");

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

  // MANUAL INSERT INTO
  table->rows[0] = malloc(sizeof(Row));
  table->rows[0]->isDeleted = false;
  table->rows[0]->values = malloc(sizeof(Value) * table->schema.columnCount);

  table->rows[0]->values[0].intValue = 1;
  table->rows[0]->values[1].boolValue = false;
  strcpy(table->rows[0]->values[2].stringValue, "first message");

  table->rows[1] = malloc(sizeof(Row));
  table->rows[1]->isDeleted = false;
  table->rows[1]->values = malloc(sizeof(Value) * table->schema.columnCount);

  table->rows[1]->values[0].intValue = 2;
  table->rows[1]->values[1].boolValue = true;
  strcpy(table->rows[1]->values[2].stringValue, "second message");

  table->rowCount = 2;
  tables->tableList[tables->tableCount++] = table;
}

void tearDown(void) {
  freeTables(tables);
}

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