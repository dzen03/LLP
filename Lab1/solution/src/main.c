#define _FILE_OFFSET_BITS 64

#include "backend.h"
#include "property.h"
#include "utils.h"

#include <stdio.h>

int main(int argc, char** argv)
{
  if (argc != 2)
    exit_with_error("Pass filename as an argument!");
  backend_start(argv[1]);


  backend_stop();

  printf("%lu", sizeof(struct property));
  return 0;
}
