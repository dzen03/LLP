#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void exit_with_error(char* message)
{
  fprintf(stderr, "Error happened, exiting. Message: %s", message);
  exit(-1);
}
