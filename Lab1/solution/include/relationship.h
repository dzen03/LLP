#ifndef LAB1_LAB1_SOLUTION_INCLUDE_RELATIONSHIP_RECORD_H_
#define LAB1_LAB1_SOLUTION_INCLUDE_RELATIONSHIP_RECORD_H_

#include <stdint.h>

enum direction
{
  BIDIRECTIONAL,
  LEFT_TO_RIGHT,
  RIGHT_TO_LEFT
};

// sizeof = 65 bytes // TODO recalculate
struct relationship
{
  enum direction direction; // <->, ->, <-
  int64_t first_node_addr; // node
  int64_t second_node_addr; // node
  int64_t relationship_type_block_addr; // dynamic_store
//  int64_t first_previous_relationship_addr; // relationship
//  int64_t first_next_relationship_addr; // relationship
//  int64_t second_previous_relationship_addr; // relationship
//  int64_t second_next_relationship_addr; // relationship
//  int64_t previous_relationship_add; // relationship
//  int64_t next_relationship_add; // relationship
} __attribute__((packed));

struct runtime_relationship
{
  enum direction direction;
  struct node* first_node_addr;
  struct node* second_node_addr;
  char* relationship_type_string;
};

#endif //LAB1_LAB1_SOLUTION_INCLUDE_RELATIONSHIP_RECORD_H_
