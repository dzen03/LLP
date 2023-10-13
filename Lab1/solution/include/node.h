#ifndef LAB1_LAB1_SOLUTION_INCLUDE_NODE_RECORD_H_
#define LAB1_LAB1_SOLUTION_INCLUDE_NODE_RECORD_H_

#include <property.h>
#include <relationship.h>

#include <stdint.h>
#include <stdio.h>

// sizeof = 16 bytes // TODO recalculate
struct node
{
  int64_t next_relationship_addr; // relationship
  int64_t next_property_addr; // property
  int64_t next_node_addr; // node
  int64_t previous_node_addr; // node
} __attribute__((packed));

struct runtime_node
{
  struct runtime_relationship* relationships;
  struct runtime_property* properties;
};

//struct node* node_read(FILE* file, int64_t addr);
//int node_write_and_free(FILE* file, int64_t addr, struct node* node);

#endif //LAB1_LAB1_SOLUTION_INCLUDE_NODE_RECORD_H_
