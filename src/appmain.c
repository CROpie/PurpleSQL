#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "repl.h"
#include "database.h"

void printHome() {
  fprintf(stderr, "db > ");
}

int appMain() {
  bool isContinue = true;

  Tables* tables = malloc(sizeof(Tables));
  tables->tablesCapacity = TABLE_CAPACITY;
  tables->tableList = malloc(sizeof(Table*) * tables->tablesCapacity);
  tables->tableCount = 0;
  
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
        tables->tableList[tables->tableCount++] = createTable(command);
        if (command->type == CMD_ERROR) {
          printf("Failed to create table:\n, %s", command->e_message);
        } else {
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

  // freeTable(table);

  return 0;
}