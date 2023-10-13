#include "backend.h"
#include "dynamic_store.h"
#include "file.h"
#include "graph.h"
#include "node.h"
#include "property.h"
#include "relationship.h"
#include "utils.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// for debug only, do not use in prod, it can and will overflow the stack
void print_graph(int64_t starting_relationship_addr)
{

}

void print_node(int64_t node_addr)
{
  struct node node;
  node_read(&node, node_addr);

  int64_t next_property_addr = node.next_property_addr;

  struct property property;
  struct dynamic_store key;
  struct dynamic_store value;
  while (next_property_addr != 0)
  {
    property_read(&property, next_property_addr);

    dynamic_store_read(&key, property.key_block_addr);

    printf("{%s: ", key.data);

    switch (property.type)
    {
      case INT:
        printf("%lld}", property.property_block.int_);
        break;
      case DOUBLE:
        printf("%f}", property.property_block.double_);
        break;
      case STRING:
        dynamic_store_read(&value, property.property_block.addr);
        printf("%s}", value.data);
        free(value.data);
        break;
      default:
        exit_with_error("Unknown type in the property value field.");
    }

    free(key.data);
    next_property_addr = property.next_property_addr;
  }
}

#define DOUBLE_EQUALITY_PRECISION 1e-5

// Returns addr in file
int64_t find_node(__attribute__((unused)) struct runtime_relationship* relationships, uint64_t relationships_count,
                  struct runtime_property* properties, uint64_t properties_count)
{
  struct file file = get_file();

  int64_t current_node_addr = file.metadata.first_node_addr;

  while (current_node_addr != 0)
  {
    struct node current_node;
    node_read(&current_node, current_node_addr);

    uint64_t need_to_find_properties = properties_count;
    uint64_t need_to_find_relationships = relationships_count;

    int64_t current_property_addr = current_node.next_property_addr;

    while (current_property_addr != 0) // searching if arguments are subset of our properties
    {
      struct property current_property;
      property_read(&current_property, current_property_addr);

      for (struct runtime_property* property = properties; property < properties + properties_count; ++property)
      {
        int equals = 1;
        if (property->type != current_property.type)
        {
          continue;
        }

        struct dynamic_store key;
        dynamic_store_read(&key, current_property.key_block_addr);
        if (strcmp((char*)key.data, property->key_string) != 0)
        {
          free(key.data);
          continue;
        }
        free(key.data);

        struct dynamic_store value;

        switch (current_property.type)
        {
          case INT:
            equals = (current_property.property_block.int_ == property->property_block.int_);
            break;
          case DOUBLE:
            equals = (fabs(current_property.property_block.double_ - property->property_block.double_) < DOUBLE_EQUALITY_PRECISION);
            break;
          case STRING:
            dynamic_store_read(&value, current_property.property_block.addr);
            equals = (strcmp((char*)value.data, property->property_block.string_) == 0);
            free(value.data);
            break;
          default:
            exit_with_error("Unknown type in the property value field.");
        }

        if(equals)
        {
          --need_to_find_properties;
          break;
        }
      }


      if (need_to_find_properties == 0 && need_to_find_relationships == 0)
      {
        return current_node_addr;
      }

      current_property_addr = current_property.next_property_addr;
    }
    current_node_addr = current_node.next_node_addr;
  }

  return -1;
}


