#ifndef LAB1_LAB1_SOLUTION_INCLUDE_DYNAMIC_STORE_H_
#define LAB1_LAB1_SOLUTION_INCLUDE_DYNAMIC_STORE_H_

#include <stdint.h>
#include <stdio.h>

#define DYNAMIC_STORE_SIZE 100 // in bytes
#define DYNAMIC_STORE_DATA_SIZE (DYNAMIC_STORE_SIZE - sizeof(struct dynamic_store_header))
#define DYNAMIC_STORE_DATA_LENGTH (DYNAMIC_STORE_DATA_SIZE / sizeof(uint8_t))

struct dynamic_store
{
  // sizeof = 16 bytes
  struct dynamic_store_header
  {
//    int64_t previous_addr; // dynamic_store
    int64_t next_addr; // dynamic_store
    uint64_t length; // in bytes
  } __attribute__((packed)) header;
  uint8_t data[DYNAMIC_STORE_SIZE - 16]; // byte array of raw data
};





#endif //LAB1_LAB1_SOLUTION_INCLUDE_DYNAMIC_STORE_H_
