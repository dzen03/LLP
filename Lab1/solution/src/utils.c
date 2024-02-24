#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#include "file.h"

static struct string_buffer utils_string_buffer = {0};

void exit_with_error(char* message)
{
  fprintf(stderr, "Error happened, exiting. Message: %s", message);
  exit(-1);
}

void string_buffer_init(void) {
#ifdef LAB1_POSIX_
  if (utils_string_buffer.open) {
    fclose(utils_string_buffer.file);
    free(utils_string_buffer.data);
    utils_string_buffer = (struct string_buffer){0};
  }
  utils_string_buffer.file = open_memstream(&utils_string_buffer.data, &utils_string_buffer.size);
  utils_string_buffer.open = 1;
#else
  #warning on non posix machine will print to stdout
#endif
}

struct string_buffer string_buffer_get(void) {
  return utils_string_buffer;
}

//void string_buffer_printf(const char* f, ...) {
//  if (utils_string_buffer.open)
//    fprintf(utils_string_buffer.file, f);
//  else
//    printf(f);
//}


