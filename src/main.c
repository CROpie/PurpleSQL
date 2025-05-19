#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// appMain is a wrapper for main to allow for unit tests of the main function itself
int appMain();

int main() {
  return appMain();
}