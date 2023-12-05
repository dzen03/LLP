#ifndef LAB1_LAB1_SOLUTION_SRC_FILE_H_
#define LAB1_LAB1_SOLUTION_SRC_FILE_H_

#include "dynamic_store.h"

#include <stdint.h>
#include <stdio.h>



int static_store_read(FILE* file, int64_t addr, void* memory, int64_t size);
int static_store_write(FILE* file, int64_t addr, const void* data, int64_t size);
int static_store_write_and_free(FILE* file, int64_t addr, void* data, int64_t size);

int dynamic_store_read_(FILE* file, int64_t addr, struct dynamic_store* store);
int dynamic_store_write_(FILE* file, int64_t addr, const struct dynamic_store* store);
//int dynamic_store_write_and_free_(FILE* file, int64_t addr, struct dynamic_store* store);

int64_t fsizeo(FILE* file);

struct file
{
  FILE* descriptor;
  struct
  {
    int64_t first_node_addr;
    int64_t last_node_addr;
    int64_t first_free_static_addr;
//    int64_t last_free_static_addr;
    int64_t first_free_dynamic_addr;
//    int64_t last_free_dynamic_addr;
  } metadata __attribute__((packed));
};

struct file file_init(char* filename);
void file_close(FILE* file);


// start of platform dependant functions
#if defined __has_include
  #if __has_include(<unistd.h>)
    #define LAB1_POSIX_
    #define _FILE_OFFSET_BITS  64
    #define _POSIX_C_SOURCE 200809L
    #define _LARGEFILE_SOURCE
  #elif __has_include(<fileapi.h>)
    #define LAB1_WINDOWS_
  #endif
#else
  #if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    #define LAB1_POSIX_
  #elif defined (_WIN64)
    #define LAB1_WINDOWS_
  #endif
#endif

#if !defined(LAB1_POSIX_) && !defined(LAB1_WINDOWS_)
  #error write your implementation of following functions
#endif

int64_t ftell64(FILE* file);
int fseek64(FILE* file, int64_t offset, int whence);
int ftrunc(FILE* file, int64_t length);

// end

#endif //LAB1_LAB1_SOLUTION_SRC_FILE_H_
