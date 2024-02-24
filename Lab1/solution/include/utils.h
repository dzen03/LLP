#ifndef LAB1_LAB1_SOLUTION_SRC_UTILS_H_
#define LAB1_LAB1_SOLUTION_SRC_UTILS_H_

//#define DEBUG
#include <stdint.h>
#include <stdio.h>

struct string_buffer {
  char* data;
  size_t size;

  FILE* file;

  int open;
};

void exit_with_error(char* message);

void string_buffer_init(void);
struct string_buffer string_buffer_get(void);

#define string_buffer_printf(f, x)           \
  if (string_buffer_get().open)              \
    fprintf(string_buffer_get().file, f, x); \
  printf(f, x);

#endif //LAB1_LAB1_SOLUTION_SRC_UTILS_H_
