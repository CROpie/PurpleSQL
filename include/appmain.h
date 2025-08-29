#ifndef MAIN_H
#define MAIN_H

# include "database.h"

typedef struct {
  int client_fd;
  Tables* tables;
} ClientArgs;

int appMain();

#endif
