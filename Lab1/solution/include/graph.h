#ifndef LAB1_LAB1_SOLUTION_SRC_GRAPH_H_
#define LAB1_LAB1_SOLUTION_SRC_GRAPH_H_

#include "property.h"
#include "relationship.h"

#include <stdint.h>

void print_graph(int64_t starting_relationship_addr);
void print_node(int64_t node_addr);


// deprecated
//int64_t find_node(struct runtime_relationship* relationships, uint64_t relationships_count,
//                  struct runtime_property* properties, uint64_t properties_count,
//                  int64_t starting_node_addr);

// Starts from node if not 0, else from the first node
// Returns addr in file
int nodes_equal(int64_t current_node_addr, struct runtime_node* runtime_node);

// Starts from node if not 0, else from the first node
// Returns addr in file
int relationship_equal(int64_t current_relationship_addr, struct runtime_relationship* runtime_relationship);

int64_t find_node(struct runtime_node* node, int64_t starting_node_addr);
int64_t find_relationship(struct runtime_relationship* relationship, int64_t starting_node_addr);

// without relationships
int add_node(struct runtime_node* node);
int add_property(struct runtime_node* node, struct runtime_property* property);
int add_relationship(struct runtime_relationship* relationship);

void remove_node(struct runtime_node* node);
void remove_property(struct runtime_node* node, struct runtime_property* property);
void remove_relationship(struct runtime_relationship* relationship);

// TODO add updates (?)


#endif //LAB1_LAB1_SOLUTION_SRC_GRAPH_H_
