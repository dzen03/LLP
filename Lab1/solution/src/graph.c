#include "backend.h"
#include "dynamic_store.h"
#include "file.h"
#include "free_space.h"
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

// Starts from node if not 0, else from the first node
// Returns addr in file or -1 if nothing found
int64_t find_node(struct runtime_node* node, int64_t starting_node_addr)
{
  struct file file = get_file();

  int64_t current_node_addr = (starting_node_addr == 0 ? file.metadata.first_node_addr : starting_node_addr);

  while (current_node_addr != 0)
  {
    struct node current_node;
    node_read(&current_node, current_node_addr);

    if (nodes_equal(&current_node, node))
      return current_node_addr;

    current_node_addr = current_node.next_node_addr;
  }

  return -1;
}

int nodes_equal(struct node* node, struct runtime_node* runtime_node)
{
  struct runtime_relationship*  relationships = runtime_node->relationships;
  uint64_t relationships_count = runtime_node->relationships_count;
  struct runtime_property* properties = runtime_node->properties;
  uint64_t properties_count = runtime_node->properties_count;

  uint64_t need_to_find_properties = properties_count;
  uint64_t need_to_find_relationships = relationships_count;

  int64_t current_property_addr = node->next_property_addr;

  while (current_property_addr != 0 && need_to_find_properties > 0) // searching if arguments are subset of our properties
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

    current_property_addr = current_property.next_property_addr;
  }

  int64_t current_relationship_addr = node->next_relationship_addr;

  while (current_relationship_addr != 0 && need_to_find_relationships > 0) // searching if arguments are subset of our properties
  {
    struct relationship current_relationship;
    relationship_read(&current_relationship, current_relationship_addr);

    for (struct runtime_relationship* relationship = relationships; relationship < relationships + relationships_count; ++relationship)
    {
      struct dynamic_store type;
      dynamic_store_read(&type, current_relationship.relationship_type_block_addr);
      if (strcmp((char*)type.data, relationship->relationship_type_string) == 0)
      {
        free(type.data);
        --need_to_find_relationships;
        break;
      }
      free(type.data);
    }

    current_relationship_addr = current_relationship.next_relationship_addr;
  }

  return (need_to_find_properties == 0 && need_to_find_relationships == 0);
}

// Starts from node if not 0, else from the first node
// Returns addr in file
int64_t find_relationship(struct runtime_relationship* relationship, int64_t starting_node_addr)
{
  int found = 0;
  int current_node_addr = starting_node_addr;
  while (!found)
  {
    current_node_addr = find_node(&relationship->first_node, current_node_addr);

    if (current_node_addr == -1)
      return -1;

    struct node node;
    node_read(&node, current_node_addr);

    int64_t current_relationship_addr = node.next_relationship_addr;

    while (current_relationship_addr != 0) // searching if arguments are subset of our properties
    {
      struct relationship current_relationship;
      relationship_read(&current_relationship, current_relationship_addr);

      struct dynamic_store type;
      dynamic_store_read(&type, current_relationship.relationship_type_block_addr);
      if (strcmp((char*)type.data, relationship->relationship_type_string) != 0)
      {
        free(type.data);
        continue;
      }
      free(type.data);

      struct node node_1, node_2;
      node_read(&node_1, current_relationship.first_node_addr);
      node_read(&node_2, current_relationship.second_node_addr);

      found = (nodes_equal(&node_1, &relationship->first_node) ||
               nodes_equal(&node_2, &relationship->first_node) ||
               nodes_equal(&node_1, &relationship->second_node) ||
               nodes_equal(&node_2, &relationship->second_node));

      if (!found)
        break;

      int inversed;

      if (nodes_equal(&node_1, &relationship->first_node) &&
          nodes_equal(&node_2, &relationship->second_node))
        inversed = 0;
      else if (nodes_equal(&node_2, &relationship->first_node) &&
          nodes_equal(&node_1, &relationship->second_node))
        inversed = 1;
      else
        continue;


      if (!inversed)
      {
        found = (relationship->direction == current_relationship.direction);
      }
      else
      {
        found = (relationship->direction == BIDIRECTIONAL && current_relationship.direction == BIDIRECTIONAL) ||
                (relationship->direction == LEFT_TO_RIGHT && current_relationship.direction == RIGHT_TO_LEFT) ||
                (relationship->direction == RIGHT_TO_LEFT && current_relationship.direction == LEFT_TO_RIGHT);
      }

      if (found)
        break;
    }
  }

  return found;
}

int add_node(struct runtime_node* node)
{
  int res = 0;
  struct runtime_property* properties = node->properties;
  int64_t properties_count = node->properties_count;

  int64_t prev_property_addr = 0;

  for (struct runtime_property* property = properties + properties_count - 1; property >= properties; --property)
  {
    struct dynamic_store key = (struct dynamic_store){{.length=strlen(property->key_string) + 1},
        (uint8_t *) property->key_string};
    int64_t key1_addr = dynamic_store_write(&key, 0);

    struct dynamic_store value;
    struct property property1;
    switch (property->type)
    {
      case STRING:
        value = (struct dynamic_store){{.length=strlen(property->property_block.string_) + 1}, (uint8_t *) property->property_block.string_};
        int64_t value1_addr = dynamic_store_write(&value, 0);
        res = (value1_addr >= 0 ? 0 : -1);

        property1 = (struct property){property->type, key1_addr, {.addr = value1_addr}, prev_property_addr};
        break;
      case INT:
        property1 = (struct property){property->type, key1_addr, {.int_ = property->property_block.int_}, prev_property_addr};
        break;
      case DOUBLE:
        property1 = (struct property){property->type, key1_addr, {.double_ = property->property_block.double_}, prev_property_addr};
        break;
    }
    if (res != 0)
      return res;

    prev_property_addr = property_write(&property1, 0);
    res = (prev_property_addr >= 0 ? 0 : -1);
  }

  struct node tmp_node = (struct node) {0, prev_property_addr};
  int64_t first_node_addr = node_write(&tmp_node, 0);

  return (first_node_addr >= 0 ? 0 : -1);
}

int remove_data(int64_t addr, int64_t size)
{
  enum free_space_type type = (size == STATIC_STORE_SIZE ? STATIC : (size == DYNAMIC_STORE_SIZE ? DYNAMIC : -1));
  struct free_space free_space = {.space = size};

  int64_t res = free_space_write(&free_space, addr, type);

  return (res == addr ? 0 : -1);
}
