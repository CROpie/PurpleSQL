#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity.h"
#include "database.h"
#include "repl.h"

void setUp(void) {
  //
}

void tearDown(void) {
}


/* Testing Select Where */
void test_parseInput_Select_star(void) {
  
  char sql[] = 
    "SELECT * FROM myTable;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_SELECT, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_TRUE(command->s_all);

  TEST_ASSERT_EQUAL_INT(1, command->s_colNameCount);
  TEST_ASSERT_EQUAL_STRING("*", command->s_colNames[0]);
  TEST_ASSERT_NULL(command->s_whereClause);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_Select_3_cols(void) {
  
  char sql[] = 
    "SELECT id, isDeleted, message FROM myTable;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_SELECT, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_FALSE(command->s_all);

  TEST_ASSERT_EQUAL_INT(3, command->s_colNameCount);
  TEST_ASSERT_EQUAL_STRING("id", command->s_colNames[0]);
  TEST_ASSERT_EQUAL_STRING("isDeleted", command->s_colNames[1]);
  TEST_ASSERT_EQUAL_STRING("message", command->s_colNames[2]);

  TEST_ASSERT_NULL(command->s_whereClause);
  
  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_Select_3_cols_nospace(void) {
  
  char sql[] = 
    "SELECT id,isDeleted,message FROM myTable;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_SELECT, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_FALSE(command->s_all);

  TEST_ASSERT_EQUAL_INT(3, command->s_colNameCount);
  TEST_ASSERT_EQUAL_STRING("id", command->s_colNames[0]);
  TEST_ASSERT_EQUAL_STRING("isDeleted", command->s_colNames[1]);
  TEST_ASSERT_EQUAL_STRING("message", command->s_colNames[2]);
  TEST_ASSERT_NULL(command->s_whereClause);
  
  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_Select_1_col(void) {
  
  char sql[] = 
    "SELECT id FROM myTable;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_SELECT, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_FALSE(command->s_all);

  TEST_ASSERT_EQUAL_INT(1, command->s_colNameCount);
  TEST_ASSERT_EQUAL_STRING("id", command->s_colNames[0]);

  TEST_ASSERT_NULL(command->s_whereClause);
  
  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_Select_star_Where_equals(void) {
  
  char sql[] = 
    "SELECT * FROM myTable WHERE id = 1;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_SELECT, command->type);

  TEST_ASSERT_EQUAL_STRING("id", command->s_whereClause->w_column);
  TEST_ASSERT_EQUAL_STRING("=", command->s_whereClause->w_operator);
  TEST_ASSERT_EQUAL_STRING("1", command->s_whereClause->w_value);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_Select_star_Where_truncated(void) {
  
  char sql[] = 
    "SELECT * FROM myTable WHE id = 1;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_ERROR, command->type);

  TEST_ASSERT_NULL(command->s_whereClause);

  // Clean up after the test 
  freeCommand(command);
}

void test_parseInput_Select_star_Where_only_col(void) {
  
  char sql[] = 
    "SELECT * FROM myTable WHERE id;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_ERROR, command->type);

  TEST_ASSERT_NULL(command->s_whereClause);

  // Clean up after the test
  freeCommand(command);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_parseInput_Select_star);
  RUN_TEST(test_parseInput_Select_3_cols);
  RUN_TEST(test_parseInput_Select_3_cols_nospace);
  RUN_TEST(test_parseInput_Select_1_col);

  RUN_TEST(test_parseInput_Select_star_Where_equals);
  RUN_TEST(test_parseInput_Select_star_Where_truncated);
  RUN_TEST(test_parseInput_Select_star_Where_only_col);

  return UNITY_END();
}
