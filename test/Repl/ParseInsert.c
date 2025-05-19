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

/* Testing InsertInto */
void test_parseInput_InsertInto_3cols_1row(void) {
  
  char sql[] = 
    "INSERT INTO myTable (id, isDeleted, message)"
    "VALUES (1, false, 'this is my message');  ";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_INSERT, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->i_numColNames);

  TEST_ASSERT_EQUAL_STRING("id", command->i_colNames[0]);
  TEST_ASSERT_EQUAL_STRING("isDeleted", command->i_colNames[1]);
  TEST_ASSERT_EQUAL_STRING("message", command->i_colNames[2]);

  TEST_ASSERT_EQUAL(1, command->i_numValueRows);

  TEST_ASSERT_EQUAL_STRING("1", command->i_colValueRows[0][0]);
  TEST_ASSERT_EQUAL_STRING("false", command->i_colValueRows[0][1]);
  TEST_ASSERT_EQUAL_STRING("this is my message", command->i_colValueRows[0][2]);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_InsertInto_3cols_2rows(void) {
  
  char sql[] = 
    "INSERT INTO myTable (id, isDeleted, message)"
    "VALUES (1, false, 'this is my message'),"
    "(2, true, 'another message');";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_INSERT, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->i_numColNames);

  TEST_ASSERT_EQUAL_STRING("id", command->i_colNames[0]);
  TEST_ASSERT_EQUAL_STRING("isDeleted", command->i_colNames[1]);
  TEST_ASSERT_EQUAL_STRING("message", command->i_colNames[2]);

  TEST_ASSERT_EQUAL(2, command->i_numValueRows);

  TEST_ASSERT_EQUAL_STRING("1", command->i_colValueRows[0][0]);
  TEST_ASSERT_EQUAL_STRING("false", command->i_colValueRows[0][1]);
  TEST_ASSERT_EQUAL_STRING("this is my message", command->i_colValueRows[0][2]);

  TEST_ASSERT_EQUAL_STRING("2", command->i_colValueRows[1][0]);
  TEST_ASSERT_EQUAL_STRING("true", command->i_colValueRows[1][1]);
  TEST_ASSERT_EQUAL_STRING("another message", command->i_colValueRows[1][2]);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_InsertInto_escaped_quotation(void) {
  
  char sql[] = 
    "INSERT INTO myTable (message)"
    "VALUES ('this isn''t my message');";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);
  TEST_ASSERT_EQUAL_STRING("this isn't my message", command->i_colValueRows[0][0]);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_InsertInto_fail_not_escaped_quotation(void) {
  
  char sql[] = 
    "INSERT INTO myTable (message)"
    "VALUES ('this isn't my message'),";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);
  TEST_ASSERT_NULL(command->i_colNames);
  TEST_ASSERT_NULL(command->i_colValueRows);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_InsertInto_WhitespaceEverywhere(void) {
  
  char sql[] = 
    "INSERT INTO myTable   (id  \n , \t isDeleted , message    )  "
    "VALUES ( 1\n  ,   false , \t'this is my message' \n \n\t)  ;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_INSERT, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->i_numColNames);

  TEST_ASSERT_EQUAL_STRING("id", command->i_colNames[0]);
  TEST_ASSERT_EQUAL_STRING("isDeleted", command->i_colNames[1]);
  TEST_ASSERT_EQUAL_STRING("message", command->i_colNames[2]);

  TEST_ASSERT_EQUAL(1, command->i_numValueRows);

  TEST_ASSERT_EQUAL_STRING("1", command->i_colValueRows[0][0]);
  TEST_ASSERT_EQUAL_STRING("false", command->i_colValueRows[0][1]);
  TEST_ASSERT_EQUAL_STRING("this is my message", command->i_colValueRows[0][2]);

  // Clean up after the test
  freeCommand(command);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_parseInput_InsertInto_3cols_1row);
  RUN_TEST(test_parseInput_InsertInto_3cols_2rows);
  RUN_TEST(test_parseInput_InsertInto_escaped_quotation);
  RUN_TEST(test_parseInput_InsertInto_fail_not_escaped_quotation);
  RUN_TEST(test_parseInput_InsertInto_WhitespaceEverywhere);

  return UNITY_END();
}
