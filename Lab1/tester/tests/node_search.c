#include "backend.h"
#include "graph.h"
#include "property.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "write_data.h"

int main(void)
{
  write("test_node_search.db");
  backend_start("test_file_read_write.db");
  struct runtime_property* properties = malloc(sizeof(struct runtime_property) * 2);

  properties[0] = (struct runtime_property){TYPE1, KEY1, {.string_ = VALUE1}};
  int64_t node1 = find_node(NULL, 0, properties, 1);

  properties[0] = (struct runtime_property){TYPE2, KEY2, {.int_ = VALUE2}};
  int64_t node2 = find_node(NULL, 0, properties, 1);

  properties[0] = (struct runtime_property){TYPE2, KEY2, {.int_ = VALUE2}};
  properties[1] = (struct runtime_property){TYPE1, KEY1, {.string_ = VALUE1}};
  int64_t node3 = find_node(NULL, 0, properties, 2);

  free(properties);

  assert(node1 == node2 && node2 == node3);

  backend_stop();
  return 0;
}
