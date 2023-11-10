#include "backend.h"

#include <assert.h>

#include "write_data.h"

int main(void)
{
  write("test_file_read_write.db");
  backend_start("test_file_read_write.db");

  __attribute__((unused)) struct file file = get_file();

  assert(file.metadata.first_node_addr != 0);
  assert(file.metadata.last_node_addr != 0);

  backend_stop();
  return 0;
}
