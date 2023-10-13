#ifndef LAB1_LAB1_SOLUTION_INCLUDE_PROPERTY_H_
#define LAB1_LAB1_SOLUTION_INCLUDE_PROPERTY_H_

#include <stdint.h>
#include <stdio.h>

// sizeof = 1 byte
enum property_type
{
  INT = 0,
  DOUBLE,
  STRING,
} __attribute__((packed));

// sizeof = 32 + sizeof property_type = 33 bytes // TODO recalculate
struct property // Key: value; key - string, value - any of property types
{
  enum property_type type;
  int64_t key_block_addr; // dynamic_store
  union
  {
    int64_t int_;
    double double_;
    int64_t addr;
  } property_block __attribute__((packed)); // either primitive type (int/double) or dynamic_store address
//  int64_t previous_property_addr; // property
  int64_t next_property_addr; // property
} __attribute__((packed));

struct runtime_property
{
  enum property_type type;
  char* key_string;
  union
  {
    int64_t int_;
    double double_;
    char* string_;
  } property_block;
};

//struct property* property_read(FILE* file, int64_t addr);
//int property_write_and_free(FILE* file, int64_t addr, struct property* property);

#endif //LAB1_LAB1_SOLUTION_INCLUDE_PROPERTY_H_
