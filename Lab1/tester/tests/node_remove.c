#include "backend.h"
#include "dynamic_store.h"
#include "free_space.h"
#include "graph.h"
#include "node.h"
#include "property.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "write_data.h"

int main(void)
{
  write("test_node_remove.db");
  backend_start("test_node_remove.db");
  struct runtime_property* properties = malloc(sizeof(struct runtime_property) * 2);
  struct runtime_node node;

  properties[0] = (struct runtime_property){TYPE1, KEY1,
                                            {.string_ = {VALUE1, strlen(VALUE1)}}};
  node = (struct runtime_node){.properties=properties, .properties_count=1, .relationships=NULL, .relationships_count=0};
  __attribute__((unused)) int64_t node1_addr = find_node(&node, 0);

  __attribute__((unused)) struct node node1;
  __attribute__((unused)) struct property property;
  assert(node_read(&node1, node1_addr) == 0);
  assert(property_read(&property, node1.next_property_addr) == 0);


  remove_node(&node);

  __attribute__((unused)) struct free_space free_space1;
  __attribute__((unused)) struct free_space free_space2;
  __attribute__((unused)) struct free_space free_space3;


  assert(free_space_read(&free_space1, node1_addr) == 0);
  assert(free_space_read(&free_space2, node1.next_property_addr) == 0);
  assert(free_space_read(&free_space3, property.key_block_addr) == 0);

  assert(free_space1.space == STATIC_STORE_SIZE);
  assert(free_space2.space == STATIC_STORE_SIZE);
  assert(free_space3.space == DYNAMIC_STORE_SIZE);

  backend_stop();
  return 0;
}
