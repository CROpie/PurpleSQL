#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "repl.h"
#include "database.h"
#include "encoder.h"

#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 12345

void printHome() {
  fprintf(stderr, "db > ");
}

int appMain() {
  bool isContinue = true;

  Tables* tables = loadTablesMetadata("purpleSQL.db");

  // set up TCP socket

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  // struct sockaddr_in address = { .sin_family = AF_INET, .sin_addr = INADDR_ANY, .sin_port = htons(12345) };
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(12345);
  address.sin_addr.s_addr = inet_addr("127.0.0.1");

  bind(server_fd, (struct sockaddr*)&address, sizeof(address));
  listen(server_fd, 3);


  printf("Server running on port 12345\n");

  
  while (isContinue) {

    int client_fd = accept(server_fd, NULL, NULL);
    printf("Accepted new connection\n");

    printHome();
    // printf("db > ");
    // char* input = getInput();
    char* input = getTCPInput(client_fd);
  
    if (!input) {
      printf("Failed to get input\n");
      isContinue = false;
    }

    printf("input: %s\n", input);

    Command* command = parseInput(input);
    switch (command->type) {

      case CMD_CREATE:
         Table* newTable = createTable(tables, command);
        if (command->type == CMD_ERROR) {
          printf("Failed to create table:\n, %s", command->e_message);
        } else {
          tables->tableList[tables->tableCount++] = newTable;
          send(client_fd, "Table creation successful.\n", 19, 0);
          printf("Table creation successful.\n");
          saveTablesMetadata(tables, "purpleSQL.db");
        }
        break;

      case CMD_INSERT:
        if (insertRecord(tables, command)) {
          printf("Record insertion successful.\n");
          saveTablesMetadata(tables, "purpleSQL.db");
        } else {
          printf("Failed to insert row. %s\n", command->e_message);
        }
        break;

      case CMD_SELECT:
        Selection* selection = selectColumns(tables, command); 

        if (command->type == CMD_ERROR) {
          printf("Failed to retrieve data. %s\n", command->e_message);
        } else {
          send(client_fd, selection->encodedString, strlen(selection->encodedString), 0);
          freeSelection(selection);
        }
        break;

      case CMD_DROP:
        if (dropTable(tables, command)) {
          printf("Table deleted.\n");
          saveTablesMetadata(tables, "purpleSQL.db");
        } else {
          printf("%s\n", command->e_message);
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
  close(client_fd);

  }
  

  // freeTable(table);

  return 0;
}