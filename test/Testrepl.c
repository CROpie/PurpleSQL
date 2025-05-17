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

/* Testing Select Where */
void test_parseInput_Select_star(void) {
  
  char sql[] = 
    "SELECT * FROM myTable;";

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

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

void test_parseInput_Select_star_Where_equals(void) {
  
  char sql[] = 
    "SELECT * FROM myTable WHERE id = 1;";

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_ERROR, command->type);

  TEST_ASSERT_NULL(command->s_whereClause);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_Select_star_Where_only_col(void) {
  
  char sql[] = 
    "SELECT * FROM myTable WHERE id;";

  Command* command = parseInput(sql);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL(CMD_ERROR, command->type);

  TEST_ASSERT_NULL(command->s_whereClause);

  // Clean up after the test
  freeCommand(command);
}

/* Testing InsertInto */
void test_parseInput_InsertInto_3cols_1row(void) {
  
  char sql[] = 
    "INSERT INTO myTable (id, isDeleted, message)"
    "VALUES (1, false, 'this is my message');";

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

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
    "VALUES ('this isn''t my message'),";

  Command* command = parseInput(sql);

  TEST_ASSERT_NOT_NULL(command);
  TEST_ASSERT_EQUAL_STRING("this isn't my message", command->i_colValueRows[0][0]);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_InsertInto_fail_not_escaped_quotation(void) {
  
  char sql[] = 
    "INSERT INTO myTable (message)"
    "VALUES ('this isn't my message'),";

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

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

/* Testing CreateTable */
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

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

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

  Command* command = parseInput(sql);

  TEST_ASSERT_NOT_NULL(command);

  TEST_ASSERT_EQUAL_STRING("", command->tableName);

  // Clean up after the test
  freeCommand(command);
}

void test_parseInput_CreateTable_fail_missing_parens(void) {
  
  char sql[] = 
    "CREATE TABLE myTable ("
    "message VARCHAR(255 );";

  Command* command = parseInput(sql);

  TEST_ASSERT_NULL(command->c_colPairs);

  // Clean up after the test
  freeCommand(command);
}

int main(void) {
  UNITY_BEGIN();
  
  RUN_TEST(test_parseInput_CreateTable_3cols);
  RUN_TEST(test_parseInput_CreateTable_3cols_extendedDef);
  RUN_TEST(test_parseInput_CreateTable_WhitespaceEverywhere);
  RUN_TEST(test_parseInput_CreateTable_3cols_extendedDef_WhitespaceEverywhere);
  RUN_TEST(test_parseInput_CreateTable_NoSpaces);
  RUN_TEST(test_parseInput_CreateTable_noTableName);
  RUN_TEST(test_parseInput_CreateTable_fail_missing_parens);

  RUN_TEST(test_parseInput_InsertInto_3cols_1row);
  RUN_TEST(test_parseInput_InsertInto_3cols_2rows);
  RUN_TEST(test_parseInput_InsertInto_escaped_quotation);
  RUN_TEST(test_parseInput_InsertInto_fail_not_escaped_quotation);
  RUN_TEST(test_parseInput_InsertInto_WhitespaceEverywhere);

  RUN_TEST(test_parseInput_Select_star);
  RUN_TEST(test_parseInput_Select_3_cols);
  RUN_TEST(test_parseInput_Select_3_cols_nospace);

  RUN_TEST(test_parseInput_Select_star_Where_equals);
  RUN_TEST(test_parseInput_Select_star_Where_truncated);
  RUN_TEST(test_parseInput_Select_star_Where_only_col);

  return UNITY_END();
}
