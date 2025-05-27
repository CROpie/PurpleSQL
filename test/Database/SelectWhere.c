#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "repl.h"
#include "database.h"
#include "testHelpers.h"

// SETUP : Manually create table, insert 2 rows
// TESTS : Write command, run SelectWhere

Tables* tables;
Command* selectCommand;
Selection* selection;
char* selectInput;

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
  if (selection) freeSelection(selection);
  freeCommand(selectCommand);
  freeTables(tables);
}


// 
void test_selectWhere_success_star(void) {
  
  char selectSql[] = 
  "SELECT * FROM myTable;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selectCommand->type, CMD_SELECT);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 2);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 1);
  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[1]->boolValue, false);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[0].values[2]->stringValue, "first message");

  TEST_ASSERT_EQUAL(selection->selectedRows[1].values[0]->intValue, 2);
  TEST_ASSERT_EQUAL(selection->selectedRows[1].values[1]->boolValue, true);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[1].values[2]->stringValue, "second message");
}

void test_selectWhere_success_id_only(void) {
  
  char selectSql[] = 
  "SELECT id FROM myTable;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selectCommand->type, CMD_SELECT);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 2);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 1);
}

void test_selectWhere_success_id_and_message(void) {
  
  char selectSql[] = 
  "SELECT id, message FROM myTable;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selectCommand->type, CMD_SELECT);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 2);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 1);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[0].values[1]->stringValue, "first message");

  TEST_ASSERT_EQUAL(selection->selectedRows[1].values[0]->intValue, 2);
  TEST_ASSERT_EQUAL_STRING(selection->selectedRows[1].values[1]->stringValue, "second message");
}

void test_selectWhere_success_id_only_where_one_id(void) {
  
  char selectSql[] = 
  "SELECT id FROM myTable WHERE id = 2;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(selectCommand->type, CMD_SELECT);

  TEST_ASSERT_EQUAL(selection->selectedRowCount, 1);

  TEST_ASSERT_EQUAL(selection->selectedRows[0].values[0]->intValue, 2);
  TEST_ASSERT_NULL(selection->selectedRows[1].values);
}

void test_selectWhere_fail_sel_unknown_col(void) {
  
  char selectSql[] = 
  "SELECT error FROM myTable WHERE id = 2;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, selectCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: unrecognized column entered", selectCommand->e_message);
}

void test_selectWhere_fail_sel_duplicate_col(void) {
  
  char selectSql[] = 
  "SELECT id, id FROM myTable WHERE id = 2;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, selectCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: duplicate column detected", selectCommand->e_message);
}

void test_selectWhere_fail_unknown_table(void) {
  
  char selectSql[] = 
  "SELECT id FROM error WHERE id = 2;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, selectCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: table not found", selectCommand->e_message);
}

void test_selectWhere_fail_where_unknown_col(void) {
  
  char selectSql[] = 
  "SELECT * FROM myTable WHERE error = 2;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, selectCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: unrecognized where column entered", selectCommand->e_message);
}

void test_selectWhere_fail_unknown_operator(void) {
  
  char selectSql[] = 
  "SELECT * FROM myTable WHERE id ? 2;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, selectCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: unrecognized operator", selectCommand->e_message);
}

void test_selectWhere_fail_where_str_instead_of_int(void) {
  
  char selectSql[] = 
  "SELECT * FROM myTable WHERE id = error;";

  selectInput = strdup(selectSql);
  selectCommand = parseInput(selectInput);

  selection = selectColumns(tables, selectCommand);

  TEST_ASSERT_EQUAL(CMD_ERROR, selectCommand->type);
  TEST_ASSERT_STRING_CONTAINS("Error: invalid input: could not convert str to int", selectCommand->e_message);
}

int main(void) {
    UNITY_BEGIN();

    // happy
    RUN_TEST(test_selectWhere_success_star);
    RUN_TEST(test_selectWhere_success_id_only);
    RUN_TEST(test_selectWhere_success_id_and_message);
    RUN_TEST(test_selectWhere_success_id_only_where_one_id);

    // unhappy
    RUN_TEST(test_selectWhere_fail_sel_unknown_col);
    RUN_TEST(test_selectWhere_fail_unknown_table);
    RUN_TEST(test_selectWhere_fail_sel_duplicate_col);
    RUN_TEST(test_selectWhere_fail_where_unknown_col);
    RUN_TEST(test_selectWhere_fail_unknown_operator);
    RUN_TEST(test_selectWhere_fail_where_str_instead_of_int);

    return UNITY_END();
}