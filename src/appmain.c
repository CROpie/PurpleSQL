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
#include "appmain.h"

#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <pthread.h>

#define PORT 12345

// global lock
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

// set up TCP socket
int startConnection(char* addr, int port) {

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = inet_addr(addr);

  bind(server_fd, (struct sockaddr*)&address, sizeof(address));
  listen(server_fd, 3);

  printf("Server running on port %d\n", port);

  return server_fd;
}

void* handle_client(void* arg) {

  ClientArgs* args = (ClientArgs*) arg;

  int client_fd = args->client_fd;
  Tables* tables = args->tables;

  bool isContinue = true;

  while (isContinue) {

    char* input = getTCPInput(client_fd);
  
    if (!input) {
      printf("Failed to get input\n");
      break;
    }

    printf("input: %s\n", input);

    Command* command = parseInput(input);

    // not as efficient as using pthread_rwlock_wrlock(&db_lock) or pthread_rwlock_rdlock(&db_lock) depending on read/write, but good enough
    // make it so a only one thread can interact with the DB at a time
    pthread_mutex_lock(&db_mutex);

    char* response = NULL;

    switch (command->type) {

      case CMD_CREATE:
         Table* newTable = createTable(tables, command);
        if (command->type == CMD_ERROR) {
          response = "{\"data\":\"Failed to create table\"}";
          printf("Failed to create table:\n, %s", command->e_message);
        } else {
          tables->tableList[tables->tableCount++] = newTable;
          response = "{\"data\":\"Table creation successful\"}";
          printf("Table creation successful.\n");
          saveTablesMetadata(tables, "purpleSQL.db");
        }
        break;

      case CMD_INSERT:
        if (insertRecord(tables, command)) {
          response = "{\"data\":\"Record insertion successful\"}";
          printf("Record insertion successful.\n");
          saveTablesMetadata(tables, "purpleSQL.db");
        } else {
          response = "{\"data\":\"Failed to insert row.\"}";
          printf("Failed to insert row. %s\n", command->e_message);
        }
        break;

      case CMD_SELECT:
        Selection* selection = selectColumns(tables, command); 

        if (command->type == CMD_ERROR) {
          response = "{\"data\":\"Failed to retrieve data.\"}";
          printf("Failed to retrieve data. %s\n", command->e_message);
        } else {
          response = strdup(selection->encodedString);  
          // send(client_fd, selection->encodedString, strlen(selection->encodedString), 0);
          freeSelection(selection);
        }
        break;

      case CMD_DROP:
        if (dropTable(tables, command)) {
          response = "{\"data\":\"Table deleted successfully\"}";
          printf("Table deleted.\n");
          saveTablesMetadata(tables, "purpleSQL.db");
        } else {
          response = "{\"data\":\"Failed to drop table.\"}";
          printf("%s\n", command->e_message);
        }
        break;

      case CMD_EXIT:
        response = "{\"data\":\"goodbyte\"}";
        printf("goodbyte\n");
        isContinue = false;
        break;
        
      case CMD_UNDEFINED:
      default:
        printf("Unrecognized command\n");
        response = "{\"data\":\"Unrecognized command\"}\n";
    }

    printf("Out of switch\n");

    if (response != NULL) {
      ssize_t n = send(client_fd, response, strlen(response), 0);
      if (n < 0) perror("send failed");
    } else {
      printf("Unable to send response\n");
    }


    freeCommand(command);
    pthread_mutex_unlock(&db_mutex);
  }

  // this will free the malloc'd args in main
  free(args);
  return NULL;
}


int appMain() {

  Tables* tables = loadTablesMetadata("purpleSQL.db");

  int server_fd = startConnection("127.0.0.1", 12345);
  
  while (true) {

    int client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0) {
      printf("accept connection failed");
      continue;
    }

    printf("Accepted new connection: %dw\n", client_fd);

    ClientArgs* args = malloc(sizeof(ClientArgs));
    args->client_fd = client_fd;
    args->tables = tables;

    pthread_t tid;

    // handle_client is recv which will block. It will return here when the socket closes
    // can only pass args to handle_client bundled into a struct
    pthread_create(&tid, NULL, handle_client, args);

    pthread_detach(tid);

  }

  close(server_fd);
  

  // freeTable(table);

  return 0;
}