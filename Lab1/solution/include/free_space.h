#ifndef LAB1_FREE_SPACE_H
#define LAB1_FREE_SPACE_H

#include <stdint.h>

// sizeof = 16 bytes // TODO recalculate
struct free_space
{
  uint64_t space; // in bytes
  int64_t next_addr; // free_space


  // TODO think about padding to STATIC_STORE_SIZE too
} __attribute__((packed));

enum free_space_type
{
  STATIC,
  DYNAMIC
};

#endif //LAB1_FREE_SPACE_H
