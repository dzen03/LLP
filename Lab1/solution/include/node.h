#ifndef LAB1_LAB1_SOLUTION_INCLUDE_NODE_RECORD_H_
#define LAB1_LAB1_SOLUTION_INCLUDE_NODE_RECORD_H_

#include <stdint.h>
#include <stdio.h>

struct node
{
  int64_t next_relationship_addr; // relationship
  int64_t next_property_addr; // property
  int64_t next_node_addr; // node
  int64_t previous_node_addr; // node

  uint8_t _[26]; // to create padding to get all static sized structs to 42 bytes // TODO check this
} __attribute__((packed));

struct runtime_node
{
  struct runtime_relationship* relationships;
  int64_t relationships_count;
  struct runtime_property* properties;
  int64_t properties_count;
};

//struct node* node_read(FILE* file, int64_t addr);
//int node_write_and_free(FILE* file, int64_t addr, struct node* node);

#endif //LAB1_LAB1_SOLUTION_INCLUDE_NODE_RECORD_H_
