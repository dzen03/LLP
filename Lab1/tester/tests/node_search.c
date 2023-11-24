#include "backend.h"
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
  write("test_node_search.db");
  backend_start("test_node_search.db");
  struct runtime_property* properties = malloc(sizeof(struct runtime_property) * 2);
  struct runtime_node node;

  properties[0] = (struct runtime_property){TYPE1, KEY1,
      {.string_ = {VALUE1, strlen(VALUE1)}}};
  node = (struct runtime_node){.properties=properties, .properties_count=1, .relationships=NULL, .relationships_count=0};
  __attribute__((unused)) int64_t node1 = find_node(&node, 0);

  properties[0] = (struct runtime_property){TYPE2, KEY2, {.int_ = VALUE2}};
  __attribute__((unused)) int64_t node2 = find_node(&node, 0);

  properties[0] = (struct runtime_property){TYPE2, KEY2, {.int_ = VALUE2}};
  properties[1] = (struct runtime_property){TYPE1, KEY1,
      {.string_ = {VALUE1, strlen(VALUE1)}}};
  node.properties_count = 2;
  __attribute__((unused)) int64_t node3 = find_node(&node, 0);

  free(properties);

  assert(node1 == node2 && node2 == node3);

  backend_stop();
  return 0;
}
