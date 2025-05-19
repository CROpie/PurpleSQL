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
  Table* table;
  
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
        table = createTable(command);
        printf("Table creation successful.\n");
        break;
      case CMD_INSERT:
        insertRecord(table, command);
        printf("Record insertion successful.\n");
        break;
      case CMD_SELECT:
        selectColumns(table, command);
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

    freeTable(table);

  return 0;
}