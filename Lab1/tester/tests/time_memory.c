#if (defined (__APPLE__) && defined (__MACH__))
#include "backend.h"
#include "file.h"
#include "graph.h"
#include "node.h"
#include "property.h"

#include <stdint.h>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>

#define THREADS

int64_t results[10000];
int64_t id[10000];


int main(int argc, __attribute__((unused)) char** argv)
{
  if (argc != 2)
    return 0;


  FILE* file = fopen("bench.csv", "w+");
  fclose(fopen("test_time_memory.db", "w")); // clear file
  fprintf(file, "number,add_ns,remove_ns,file_size,memory\n");


  struct runtime_property prop;
  struct runtime_node node = {.properties_count=1, .properties=&prop};

  uint64_t add_ns, remove_ns;
  uint64_t file_size;
  uint64_t memory;


  for(int i = 0; i < 1000; ++i)
  {
    backend_start("test_time_memory.db");
    uint64_t start = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
    for (int j = 1; j <= i; ++j)
    {
      prop = (struct runtime_property){.property_block.int_=j, .type=INT, .key_string="index"};
      add_node(&node);
    }
    uint64_t finish = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
    add_ns = finish - start;

    file_size = fsizeo(get_file().descriptor);

    start = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
    for (int j = 1; j <= i; ++j)
    {
      prop = (struct runtime_property){.property_block.int_=j, .type=INT, .key_string="index"};
      remove_node(&node);
    }
    finish = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
    remove_ns = finish - start;

    memory = 0;
    struct rusage r;
    if (getrusage(RUSAGE_SELF, &r) == 0)
      memory = r.ru_maxrss;

    fprintf(file, "%d,%llu,%llu,%llu,%llu\n", i, add_ns, remove_ns, file_size, memory);
    backend_stop();
  }

  return 0;
}
#else
int main(void)
{
  return 0;
}
#endif
