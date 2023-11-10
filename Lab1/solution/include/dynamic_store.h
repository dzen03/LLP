#ifndef LAB1_LAB1_SOLUTION_INCLUDE_DYNAMIC_STORE_H_
#define LAB1_LAB1_SOLUTION_INCLUDE_DYNAMIC_STORE_H_

#include <stdint.h>
#include <stdio.h>


struct dynamic_store
{
  // sizeof = 24 bytes // TODO recalculate
  struct dynamic_store_header
  {
//    int64_t previous_addr; // dynamic_store
    int64_t next_addr; // dynamic_store
    uint64_t length; // in bytes
  } __attribute__((packed)) header;
  uint8_t* data; // byte array of raw data
};

#define DYNAMIC_STORE_SIZE 4096 // in bytes



#endif //LAB1_LAB1_SOLUTION_INCLUDE_DYNAMIC_STORE_H_
