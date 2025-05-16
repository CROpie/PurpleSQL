#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "repl.h"
#include "database.h"

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
