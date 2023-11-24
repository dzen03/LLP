#include "backend.h"
#include "dynamic_store.h"
#include "utils.h"

#include <stdio.h>



int main(int argc, char** argv)
{
  if (argc != 2)
    exit_with_error("Pass filename as an argument!");
  backend_start(argv[1]);

  printf("%d %lu %lu", DYNAMIC_STORE_SIZE, DYNAMIC_STORE_DATA_SIZE, DYNAMIC_STORE_DATA_LENGTH);


  backend_stop();
  return 0;
}
