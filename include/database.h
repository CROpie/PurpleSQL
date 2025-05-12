#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

typedef enum {
    CMD_CREATE,
    CMD_INSERT,
    CMD_SELECT,
    CMD_UNDEFINED,
    CMD_EXIT,
    CMD_ERROR
} CommandType;

typedef enum {
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,
} Operand;

typedef enum {
  COL_INT,
  COL_BOOL,
  COL_STRING,
  COL_UNDEFINED
} ColumnType;

typedef struct {
  char name[32];
  ColumnType type;
} ColumnSchema;

typedef struct {
  ColumnSchema* columns;
  int columnCount;
} TableSchema;

typedef union {
  int intValue;
  bool boolValue;
  char stringValue[255];
} Value;

typedef struct {
  Value* values;
} Row;

typedef struct {
  TableSchema schema;
  Row** rows;
  int rowCount;
} Table;

typedef struct {
    char* column;
    Operand op;
    char* value;
} Condition;

typedef struct {
    CommandType type;
    char* tableName;

    // CREATE
    int columnCount;
    char** columnNames;
    ColumnType* columnTypes;

    // INSERT
    int valueCount;
    char** insertValues;

    // SELECT
    bool selectAll;
    int selectCount;
    char** selectColumns;

    // WHERE (only one condition for now)
   //  Condition* condition;
} Command;

Table* createTable(Command* command);
#endif
