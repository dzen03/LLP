#include "backend.h"
#include "graph.h"
#include "node.h"
#include "property.h"

#include <stdio.h>

// too long to run each time
int main(int argc, __attribute__((unused)) char** argv)
{
  if (argc == 1)
    return 0;

  fclose(fopen("test_huge_file_write.db", "w")); // clear file

  struct runtime_property prop;
  struct runtime_node node = {.properties_count=1, .properties=&prop};

  backend_start("test_huge_file_write.db");
  for(int i = 0; i < 3000000; ++i)
  {
    prop = (struct runtime_property){.property_block.int_=i, .type=INT, .key_string="index"};
    add_node(&node);
  }
  backend_stop();

  return 0;
}
