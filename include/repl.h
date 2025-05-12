#ifndef REPL_H
#define REPL_H

#include "database.h"

#define INPUT_LENGTH 512
#define COL_CAPACITY 4

char* getInput();
Command* parseInput(char* input);

#endif
