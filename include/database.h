#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

#define ROW_CAPACITY 4
#define SELECT_CAPACITY 4
#define TABLE_CAPACITY 4
#define PAGE_SIZE 4096
#define PAGE_CAPACITY 1
#define VARCHAR_LENGTH 64

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
  int rowsPerPage;
} TableSchema;

typedef union {
  int intValue;
  bool boolValue;
  char stringValue[VARCHAR_LENGTH];
} Value;

/* Row -> Record instead? This will do for now*/
typedef struct {
  Value** values;
  int valueCount;
} SelectedRow;

typedef struct {
  SelectedRow* selectedRows;
  int columnTypeCount;
  int selectedRowCount;
  int selectCapacity;
} Selection;

typedef struct {
  int columnIndex;
  ColumnType columnType;
} SelectColumnInfo;

typedef struct {
  bool isDeleted;
  Value values[];
} Row;

typedef struct {
  __uint8_t data[PAGE_SIZE];
} Page;

typedef struct {
  char* name;
  TableSchema schema;
  int rowCount;
  Page** pages;
  int pageCount;
  int pageCapacity;
} Table;

typedef struct {
  Table** tableList;
  int tableCount;
  int tablesCapacity;
} Tables;

typedef struct {
  char* colName;
  char* colDef;
} ColPair;

typedef struct {
    char* w_column;
    char* w_operator;
    char* w_value;
} Where_Clause;

typedef struct {
    CommandType type;
    char* tableName;

    // CREATE
    int c_numColPairs;
    ColPair* c_colPairs;

    // INSERT
    int i_numColNames;
    char** i_colNames;
    int i_numValueRows;
    char*** i_colValueRows;

    // SELECT
    bool s_all;
    int s_colNameCount;
    char** s_colNames;

    // WHERE
    Where_Clause* s_whereClause;

    // ERROR
    char* e_message;
} Command;

Table* createTable(Command* command);
bool insertRecord(Tables* tables, Command* command);
Selection* selectColumns(Tables* tables, Command* command);
void freeTable(Table* table);
void freeTables(Tables* tables);
void freeSelection(Selection* selection);

#endif
