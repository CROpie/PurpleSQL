#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "repl.h"
#include "database.h"

void freeCommand(Command* command) {
  if (!command) return;

  if (command->tableName) free(command->tableName);

  if (command->columnNames) {
    for (int i = 0; i < command->columnCount; i++) {
      free(command->columnNames[i]);
    }
    free(command->columnNames);
  }

  if (command->columnTypes) free(command->columnTypes);

  if (command->insertValues) {
    for (int i = 0; i < command->valueCount; i++) {
      free(command->insertValues[i]);
    }
  }

  if (command->selectColumns) {
    for (int i = 0; i < command->selectCount; i++) {
      free(command->selectColumns[i]);
    }
    free(command->selectColumns);
  }
  free(command);
}

int main() {
  bool isContinue = true;
  
  while (isContinue) {
    printf("db > ");
    char* input = getInput();
  
    if (!input) {
      printf("Failed to get input\n");
      isContinue = false;
    }

    Command* command = parseInput(input);
    switch (command->type) {
      case CMD_CREATE:
        printf("creating table...\n");
        break;
      case CMD_EXIT:
        printf("goodbyte\n");
        isContinue = false;
        break;
      case CMD_UNDEFINED:
      default:
      printf("Unrecognized command\n");
    }

    // if (input) free(input);
    freeCommand(command);
  }

  return 0;
}
