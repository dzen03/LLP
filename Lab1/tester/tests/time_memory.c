#if (defined (__APPLE__) && defined (__MACH__))
#define _DARWIN_C_SOURCE

#include "backend.h"
#include "file.h"
#include "graph.h"
#include "node.h"
#include "property.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

//not cross platform TODO fix it
int main(int argc, __attribute__((unused)) char** argv)
{
  if (argc != 2)
    return 0;


  FILE* file = fopen("bench.csv", "w+");
  FILE* data = fopen("test.txt", "r");
  fclose(fopen("test_time_memory.db", "w")); // clear file
  fprintf(file, "add,remove,memory,file_size\n");


  struct runtime_property prop;
  struct runtime_node node = {.properties_count=1, .properties=&prop};

  uint64_t add_ns, remove_ns;
  uint64_t file_size;
  uint64_t memory;

  int arr[400];
  int ind = 0;


  for(int i = 0; i < 1000; ++i)
  {

    backend_start("test_time_memory.db");
    for (int j = ind; j < ind + 500;)
    {
      uint64_t start = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
      for (int k = j; j <= k + 100; ++j)
      {
        prop = (struct runtime_property){.property_block.int_=j, .type=INT, .key_string="index"};
        add_node(&node);
      }
      uint64_t finish = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
      add_ns = finish - start;
      file_size = fsizeo(get_file().descriptor);

      memory = 0;
      struct rusage r;
      if (getrusage(RUSAGE_SELF, &r) == 0)
        memory = r.ru_maxrss;

      fprintf(file, "%llu\n", add_ns);
      fprintf(file, ",,,%llu\n", file_size);
      fprintf(file, ",,%llu\n", memory);
    }
    ind += 500;

    char buf[3000];
    fgets(buf, 3000, data);
    char* p = buf;
    for (int j = 0; j < 400; ++j)
    {
      arr[j] = (int)strtol(p, &p, 10);
      ++p;
    }

    for (int j = 0; j < 400;)
    {
      uint64_t start = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
      for (int k = j; j <= k + 100; ++j)
      {
        prop = (struct runtime_property){.property_block.int_=arr[j], .type=INT, .key_string="index"};
        remove_node(&node);
      }
      uint64_t finish = clock_gettime_nsec_np(CLOCK_PROCESS_CPUTIME_ID);
      remove_ns = finish - start;
      fprintf(file, ",%llu\n", remove_ns);
      file_size = fsizeo(get_file().descriptor);

      memory = 0;
      struct rusage r;
      if (getrusage(RUSAGE_SELF, &r) == 0)
        memory = r.ru_maxrss;

      fprintf(file, ",,,%llu\n", file_size);
      fprintf(file, ",,%llu\n", memory);
    }




//    fprintf(file, "%d,%llu,%llu,%llu,%llu\n", i, add_ns, remove_ns, file_size, memory);
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
