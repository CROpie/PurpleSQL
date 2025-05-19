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

/* Testing CreateTable */
void test_parseInput_CreateTable_3cols(void) {
  
  char sql[] = 
    "CREATE TABLE myTable (\n"
    "id INT,\n"
    "isDeleted BOOL,\n"
    "message VARCHAR(255) );";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_CREATE, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->c_numColPairs);

  TEST_ASSERT_EQUAL_STRING("id", command->c_colPairs[0].colName);
  TEST_ASSERT_EQUAL_STRING("INT", command->c_colPairs[0].colDef);

  TEST_ASSERT_EQUAL_STRING("isDeleted", command->c_colPairs[1].colName);
  TEST_ASSERT_EQUAL_STRING("BOOL", command->c_colPairs[1].colDef);

  TEST_ASSERT_EQUAL_STRING("message", command->c_colPairs[2].colName);
  TEST_ASSERT_EQUAL_STRING("VARCHAR(255)", command->c_colPairs[2].colDef);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_3cols_extendedDef(void) {
  
  char sql[] = 
    "CREATE TABLE myTable (\n"
    "id INT NOT NULL AUTO_INCREMENT,\n"
    "isDeleted BOOL,\n"
    "message VARCHAR(255) NOT NULL);";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_CREATE, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->c_numColPairs);

  TEST_ASSERT_EQUAL_STRING("id", command->c_colPairs[0].colName);
  TEST_ASSERT_EQUAL_STRING("INT NOT NULL AUTO_INCREMENT", command->c_colPairs[0].colDef);

  TEST_ASSERT_EQUAL_STRING("isDeleted", command->c_colPairs[1].colName);
  TEST_ASSERT_EQUAL_STRING("BOOL", command->c_colPairs[1].colDef);

  TEST_ASSERT_EQUAL_STRING("message", command->c_colPairs[2].colName);
  TEST_ASSERT_EQUAL_STRING("VARCHAR(255) NOT NULL", command->c_colPairs[2].colDef);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_WhitespaceEverywhere(void) {
  
  char sql[] = 
    "CREATE TABLE myTable    \t  (\n"
    "   id    INT  \n  \t ,\n"
    "   isDeleted \t   \n  BOOL,   \n"
    "\n  \t  message   VARCHAR(255)    )   \t\n\t   ;";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_CREATE, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->c_numColPairs);

  TEST_ASSERT_EQUAL_STRING("id", command->c_colPairs[0].colName);
  TEST_ASSERT_EQUAL_STRING("INT", command->c_colPairs[0].colDef);

  TEST_ASSERT_EQUAL_STRING("isDeleted", command->c_colPairs[1].colName);
  TEST_ASSERT_EQUAL_STRING("BOOL", command->c_colPairs[1].colDef);

  TEST_ASSERT_EQUAL_STRING("message", command->c_colPairs[2].colName);
  TEST_ASSERT_EQUAL_STRING("VARCHAR(255)", command->c_colPairs[2].colDef);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_3cols_extendedDef_WhitespaceEverywhere(void) {
  
  char sql[] = 
    "CREATE TABLE myTable    (\n"
    "id    INT  \n \t NOT  \t NULL AUTO_INCREMENT,\n"
    "isDeleted    BOOL   ,  \n"
    "message \t\n VARCHAR(255)  \n NOT   \t NULL );";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_CREATE, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->c_numColPairs);

  TEST_ASSERT_EQUAL_STRING("id", command->c_colPairs[0].colName);
  TEST_ASSERT_EQUAL_STRING("INT NOT NULL AUTO_INCREMENT", command->c_colPairs[0].colDef);

  TEST_ASSERT_EQUAL_STRING("isDeleted", command->c_colPairs[1].colName);
  TEST_ASSERT_EQUAL_STRING("BOOL", command->c_colPairs[1].colDef);

  TEST_ASSERT_EQUAL_STRING("message", command->c_colPairs[2].colName);
  TEST_ASSERT_EQUAL_STRING("VARCHAR(255) NOT NULL", command->c_colPairs[2].colDef);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_NoSpaces(void) {
  
  char sql[] = 
    "CREATE TABLE myTable(id INT,isDeleted BOOL,message VARCHAR(255));";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_CREATE, command->type);

  TEST_ASSERT_EQUAL_STRING("myTable", command->tableName);

  TEST_ASSERT_EQUAL(3, command->c_numColPairs);

  TEST_ASSERT_EQUAL_STRING("id", command->c_colPairs[0].colName);
  TEST_ASSERT_EQUAL_STRING("INT", command->c_colPairs[0].colDef);

  TEST_ASSERT_EQUAL_STRING("isDeleted", command->c_colPairs[1].colName);
  TEST_ASSERT_EQUAL_STRING("BOOL", command->c_colPairs[1].colDef);

  TEST_ASSERT_EQUAL_STRING("message", command->c_colPairs[2].colName);
  TEST_ASSERT_EQUAL_STRING("VARCHAR(255)", command->c_colPairs[2].colDef);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_noTableName(void) {
  
  char sql[] = 
    "CREATE TABLE (\n"
    "id INT,\n"
    "isDeleted BOOL,\n"
    "message VARCHAR(255) );";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL_STRING("", command->tableName);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_no_trailing_semicolon(void) {
  
  char sql[] = 
    "CREATE TABLE myTable ("
    "message VARCHAR( 255 )";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_EQUAL(CMD_ERROR, command->type);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_fail_missing_parens(void) {
  
  char sql[] = 
    "CREATE TABLE myTable ("
    "message VARCHAR(255 );";

  char* input = strdup(sql);
  Command* command = parseInput(input);

  TEST_ASSERT_NULL(command->c_colPairs);

  // Clean up after the test
  freeCommand(command);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_parseInput_CreateTable_no_trailing_semicolon);
  
  RUN_TEST(test_parseInput_CreateTable_3cols);
  RUN_TEST(test_parseInput_CreateTable_3cols_extendedDef);
  RUN_TEST(test_parseInput_CreateTable_WhitespaceEverywhere);
  RUN_TEST(test_parseInput_CreateTable_3cols_extendedDef_WhitespaceEverywhere);
  RUN_TEST(test_parseInput_CreateTable_NoSpaces);
  RUN_TEST(test_parseInput_CreateTable_noTableName);
  RUN_TEST(test_parseInput_CreateTable_fail_missing_parens);

  return UNITY_END();
}
