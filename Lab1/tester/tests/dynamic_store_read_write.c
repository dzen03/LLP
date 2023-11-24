#include "backend.h"
#include "dynamic_store.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(void)
{

  fclose(fopen("test_dynamic_store_read_write.db", "w")); // clear file

  backend_start("test_dynamic_store_read_write.db");

#define MEM_SIZE (DYNAMIC_STORE_DATA_SIZE * 10)

  uint8_t* mem = malloc(MEM_SIZE);

  int64_t mem_addr = dynamic_store_write_chain((uint8_t*) mem, MEM_SIZE, 0);

  uint8_t* mem_r;
  uint64_t mem_r_size = dynamic_store_read_chain(&mem_r, mem_addr);

  assert(mem_r_size == MEM_SIZE);
  assert(memcmp(mem, mem_r, MEM_SIZE) == 0);

  free(mem);
  free(mem_r);

  backend_stop();
  return 0;
}
