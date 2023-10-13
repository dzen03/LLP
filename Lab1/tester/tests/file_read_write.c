#include "backend.h"
#include "file.h"
#include "graph.h"
#include "relationship.h"

#include <assert.h>

#include "write_data.h"

int main(void)
{
  write("test_file_read_write.db");
  backend_start("test_file_read_write.db");

  struct file file = get_file();

  assert(file.metadata.first_node_addr != 0);
  assert(file.metadata.first_relationship_addr != 0);
  assert(file.metadata.last_node_addr != 0);
  assert(file.metadata.last_relationship_addr != 0);

  struct relationship relationship;
  relationship_read(&relationship, file.metadata.first_relationship_addr);

  print_node(relationship.first_node_addr);
  print_node(relationship.second_node_addr);


  backend_stop();
  return 0;
}
