#ifndef LAB1_LAB1_SOLUTION_SRC_GRAPH_H_
#define LAB1_LAB1_SOLUTION_SRC_GRAPH_H_

#include "property.h"
#include "relationship.h"

#include <stdint.h>

void print_graph(int64_t starting_relationship_addr);
void print_node(int64_t node_addr);



int64_t find_node(__attribute__((unused)) struct runtime_relationship* relationships, uint64_t relationships_count,
                  struct runtime_property* properties, uint64_t properties_count);

#endif //LAB1_LAB1_SOLUTION_SRC_GRAPH_H_
