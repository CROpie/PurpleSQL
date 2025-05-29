#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "repl.h"
#include "database.h"
#include "encoder.h"

void printHome() {
  fprintf(stderr, "db > ");
}

int appMain() {
  bool isContinue = true;

  Tables* tables = loadTablesMetadata("purpleSQL.db");
  
  while (isContinue) {
    printHome();
    // printf("db > "); 
    char* input = getInput();
  
    if (!input) {
      printf("Failed to get input\n");
      isContinue = false;
    }

    Command* command = parseInput(input);
    switch (command->type) {

      case CMD_CREATE:
         Table* newTable = createTable(tables, command);
        if (command->type == CMD_ERROR) {
          printf("Failed to create table:\n, %s", command->e_message);
        } else {
          tables->tableList[tables->tableCount++] = newTable;
          printf("Table creation successful.\n");
        }
        break;

      case CMD_INSERT:
        if (insertRecord(tables, command)) {
          printf("Record insertion successful.\n");
        } else {
          printf("Failed to insert row:\n, %s", command->e_message);
        }
        break;

      case CMD_SELECT:
        Selection* selection = selectColumns(tables, command);
        if (!selection) {
          // do stg
        } else {
          freeSelection(selection);
        }

        break;

      case CMD_EXIT:
        printf("goodbyte\n");
        isContinue = false;
        break;
        
      case CMD_UNDEFINED:
      default:
        printf("Unrecognized command\n");
    }

  freeCommand(command);
  }
  saveTablesMetadata(tables, "purpleSQL.db");

  // freeTable(table);

  return 0;
}