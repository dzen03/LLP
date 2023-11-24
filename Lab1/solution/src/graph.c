#include "backend.h"
#include "dynamic_store.h"
#include "file.h"
//#include "free_space.h"
#include "graph.h"
#include "node.h"
#include "property.h"
#include "relationship.h"
#include "utils.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
//#include <stdlib.h>
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
        break;
      default:
        exit_with_error("Unknown type in the property value field.");
    }

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

    if (nodes_equal(current_node_addr, node))
      return current_node_addr;

    current_node_addr = current_node.next_node_addr;
  }

  return -1;
}


int nodes_equal(int64_t current_node_addr, struct runtime_node* runtime_node)
{
  struct node node;
  node_read(&node, current_node_addr);
  struct runtime_relationship*  relationships = runtime_node->relationships;
  uint64_t relationships_count = runtime_node->relationships_count;
  struct runtime_property* properties = runtime_node->properties;
  uint64_t properties_count = runtime_node->properties_count;

  uint64_t need_to_find_properties = properties_count;
  uint64_t need_to_find_relationships = relationships_count;

  int64_t current_property_addr = node.next_property_addr;

  while (current_property_addr != 0 && need_to_find_properties > 0) // searching if arguments are subset of our properties
  {
    struct property current_property;
    property_read(&current_property, current_property_addr);

    for (struct runtime_property* property = properties; property < properties + properties_count; ++property)
    {
      int equals = 1;


      if(equals)
      {
        --need_to_find_properties;
        break;
      }
    }

    current_property_addr = current_property.next_property_addr;
  }

  int64_t current_relationship_addr = node.next_relationship_addr;

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
        --need_to_find_relationships;
        break;
      }
    }

    current_relationship_addr = (current_relationship.first_node_addr == current_node_addr ?
        current_relationship.first_next_relationship_addr : current_relationship.second_next_relationship_addr);
  }

  return (need_to_find_properties == 0 && need_to_find_relationships == 0);
}

int relationship_equal(int64_t current_relationship_addr, struct runtime_relationship* runtime_relationship)
{
  struct relationship current_relationship;
  relationship_read(&current_relationship, current_relationship_addr);

  struct dynamic_store type;
  dynamic_store_read(&type, current_relationship.relationship_type_block_addr);
  if (strcmp((char*)type.data, runtime_relationship->relationship_type_string) != 0)
  {
    return 0;
  }

  struct node node_1, node_2;
  node_read(&node_1, current_relationship.first_node_addr);
  node_read(&node_2, current_relationship.second_node_addr);

  int inversed;

  if (nodes_equal(current_relationship.first_node_addr, &runtime_relationship->first_node) &&
      nodes_equal(current_relationship.second_node_addr, &runtime_relationship->second_node))
    inversed = 0;
  else if (nodes_equal(current_relationship.second_node_addr, &runtime_relationship->first_node) &&
      nodes_equal(current_relationship.first_node_addr, &runtime_relationship->second_node))
    inversed = 1;
  else
    return 0;

  int found = 0;

  if (!inversed)
    found = (runtime_relationship->direction == current_relationship.direction);
  else
    found = (runtime_relationship->direction == BIDIRECTIONAL && current_relationship.direction == BIDIRECTIONAL) ||
        (runtime_relationship->direction == LEFT_TO_RIGHT && current_relationship.direction == RIGHT_TO_LEFT) ||
        (runtime_relationship->direction == RIGHT_TO_LEFT && current_relationship.direction == LEFT_TO_RIGHT);

  return found;
}

int property_equal(int64_t current_property_addr, struct runtime_property* runtime_property)
{
  struct property current_property;
  property_read(&current_property, current_property_addr);

  if (runtime_property->type != current_property.type)
  {
    return 0;
  }

  struct dynamic_store key;
  dynamic_store_read(&key, current_property.key_block_addr);
  if (strcmp((char*)key.data, runtime_property->key_string) != 0)
  {
    return 0;
  }

  struct dynamic_store value;
  int equals;

  switch (current_property.type)
  {
    case INT:
      equals = (current_property.property_block.int_ == runtime_property->property_block.int_);
      break;
    case DOUBLE:
      equals = (fabs(current_property.property_block.double_ - runtime_property->property_block.double_) < DOUBLE_EQUALITY_PRECISION);
      break;
    case STRING:
      dynamic_store_read(&value, current_property.property_block.addr);
      equals = (strcmp((char*)value.data, runtime_property->property_block.string_.data) == 0);
      break;
    default:
      exit_with_error("Unknown type in the property value field.");
  }

  return equals;
}

// Starts from node if not 0, else from the first node
// Returns addr in file
int64_t find_relationship(struct runtime_relationship* relationship, int64_t starting_node_addr)
{
  int64_t current_node_addr = starting_node_addr;
  int64_t current_relationship_addr = -2;
  while (current_node_addr != 0)
  {
    current_node_addr = find_node(&relationship->first_node, current_node_addr);

    if (current_node_addr == -1)
      return -1;

    struct node node;
    node_read(&node, current_node_addr);

    current_relationship_addr = node.next_relationship_addr;

    while (current_relationship_addr != 0) // searching if arguments are subset of our properties
    {
      if (relationship_equal(current_relationship_addr, relationship))
        return current_relationship_addr;
    }
  }

  return -2;
}

int add_node(struct runtime_node* node)
{
  int res = 0;
  struct runtime_property* properties = node->properties;
  int64_t properties_count = node->properties_count;

  int64_t prev_property_addr = 0;

  for (struct runtime_property* property = properties + properties_count - 1; property >= properties; --property)
  {
    int64_t key1_addr = dynamic_store_write_chain((uint8_t*)property->key_string, strlen(property->key_string) + 1, 0);

    int64_t value1_addr;
    struct property property1;
    switch (property->type)
    {
      case STRING:
        value1_addr = dynamic_store_write_chain((uint8_t*)property->property_block.string_.data,
                                                        property->property_block.string_.length, 0);
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

int add_property(struct runtime_node* node, struct runtime_property* property)
{
  int64_t node_addr = find_node(node, 0);

  if (node_addr == -1)
    return -1;

  struct node node_;
  node_read(&node_, node_addr);

  int64_t key1_addr = dynamic_store_write_chain((uint8_t*)property->key_string, strlen(property->key_string) + 1, 0);

  int64_t value1_addr;
  struct property property1;
  int res = 0;

  switch (property->type)
  {
    case STRING:
      value1_addr = dynamic_store_write_chain((uint8_t*)property->property_block.string_.data,
                                              property->property_block.string_.length, 0);
      res = (value1_addr >= 0 ? 0 : -1);

      property1 = (struct property){property->type, key1_addr, {.addr = value1_addr}, node_.next_property_addr};
      break;
    case INT:
      property1 = (struct property){property->type, key1_addr, {.int_ = property->property_block.int_}, node_.next_property_addr};
      break;
    case DOUBLE:
      property1 = (struct property){property->type, key1_addr, {.double_ = property->property_block.double_}, node_.next_property_addr};
      break;
  }
  if (res != 0)
    return res;

  node_.next_property_addr = property_write(&property1, 0);
  res = (node_.next_property_addr >= 0 ? 0 : -1);

  if (res != 0)
    return res;

  res = (node_write(&node_, node_addr) == node_addr);

  return res;
}

int add_relationship(struct runtime_relationship* relationship)
{
  int64_t node1_addr = find_node(&relationship->first_node, 0);
  int64_t node2_addr = find_node(&relationship->second_node, 0);

  if (node1_addr <= 0 || node2_addr <= 0)
    return -1;

  struct node node1, node2;

  node_read(&node1, node1_addr);
  node_read(&node2, node2_addr);

  int64_t relationship_type_addr = dynamic_store_write_chain((uint8_t*) relationship->relationship_type_string,
                                                        strlen(relationship->relationship_type_string) + 1, 0);
  if (relationship_type_addr <= 0)
    return -2;

  struct relationship relationship_ = {.direction=relationship->direction, .relationship_type_block_addr=relationship_type_addr,
      .first_node_addr=node1_addr, .second_node_addr=node2_addr,
      .first_next_relationship_addr=node1.next_relationship_addr, .second_next_relationship_addr=node2.next_relationship_addr};

  int64_t relationship_addr =  relationship_write(&relationship_, 0);

  if (relationship_addr <= 0)
    return -3;

  struct relationship first;
  relationship_read(&first, node1.next_relationship_addr);
  first.first_previous_relationship_addr = relationship_addr;

  if (relationship_write(&first, node1.next_relationship_addr) <= 0)
    return -4;

  struct relationship second;
  relationship_read(&second, node2.next_relationship_addr);
  second.first_previous_relationship_addr = relationship_addr;

  if (relationship_write(&second, node2.next_relationship_addr) <= 0)
    return -4;


  node1.next_relationship_addr = relationship_addr;
  node2.next_relationship_addr = relationship_addr;

  if (node_write(&node1, node1_addr) <= 0 || node_write(&node2, node2_addr) <= 0)
    return -5;

  return 0;
}

void remove_node(struct runtime_node* node)
{
  int64_t node_addr = find_node(node, 0);

  struct node node_;
  node_read(&node_, node_addr);


  if (node_.previous_node_addr != 0)
  {
    struct node prev;
    node_read(&prev, node_.previous_node_addr);

    prev.next_node_addr = node_.next_node_addr;

    node_write(&prev, node_.previous_node_addr);
  }
  if (node_.next_node_addr != 0)
  {
    struct node next;
    node_read(&next, node_.next_node_addr);

    next.previous_node_addr = node_.previous_node_addr;

    node_write(&next, node_.next_node_addr);
  }

  int need_update = 0;
  struct file f = get_file();
  if (f.metadata.first_node_addr == node_addr)
    f.metadata.first_node_addr = node_.next_node_addr, need_update = 1;
  if (f.metadata.last_node_addr == node_addr)
    f.metadata.last_node_addr = node_.previous_node_addr, need_update = 1;

  if (need_update)
    update_metadata(f);

  int64_t current_property_addr = node_.next_property_addr;

  while (current_property_addr != 0)
  {
    struct property current_property;
    property_read(&current_property, current_property_addr);

    dynamic_store_remove(current_property.key_block_addr);
    if (current_property.type == STRING)
      dynamic_store_remove(current_property.property_block.addr);
    // TODO add your allocated types here!

    int64_t temp = current_property_addr;
    current_property_addr = current_property.next_property_addr;

    remove_data(temp, sizeof(struct property));
  }

  int64_t current_relationship_addr = node_.next_relationship_addr;

  while (current_relationship_addr != 0)
  {
    struct relationship current_relationship;
    relationship_read(&current_relationship, current_relationship_addr);

    dynamic_store_remove(current_relationship.relationship_type_block_addr);

    int64_t tmp = current_relationship_addr;

    current_relationship_addr = (current_relationship.first_node_addr == node_addr ?
                                 current_relationship.first_next_relationship_addr : current_relationship.second_next_relationship_addr);

    remove_data(tmp, sizeof(struct relationship));
  }

  remove_data(node_addr, sizeof(struct node));
}

void remove_property(struct runtime_node* node, struct runtime_property* property)
{
  int64_t node_addr = find_node(node, 0);

  struct node node_;
  node_read(&node_, node_addr);

  int64_t previous_property_addr = 0;
  int64_t current_property_addr = node_.next_property_addr;

  while (current_property_addr != 0)
  {
    struct property current_property;
    property_read(&current_property, current_property_addr);

    if (property->type != current_property.type)
    {
      previous_property_addr = current_property_addr;
      current_property_addr = current_property.next_property_addr;
      continue;
    }

    struct dynamic_store key;
    dynamic_store_read(&key, current_property.key_block_addr);
    if (strcmp((char*)key.data, property->key_string) != 0)
    {
      previous_property_addr = current_property_addr;
      current_property_addr = current_property.next_property_addr;
      continue;
    }

    // found

    if (previous_property_addr != 0)
    {
      struct property prev;
      property_read(&prev, previous_property_addr);

      prev.next_property_addr = current_property.next_property_addr;
      property_write(&prev, previous_property_addr);
    }

    if (current_property.type == STRING)
    {
      dynamic_store_remove(current_property.property_block.addr);
    }

    dynamic_store_remove(current_property.key_block_addr);

    remove_data(current_property_addr, sizeof(struct property));

    break;
  }
}

void remove_relationship(struct runtime_relationship* relationship)
{
  int64_t relationship_addr = find_relationship(relationship, 0);
  struct relationship relationship_;
  relationship_read(&relationship_, relationship_addr);

  if (relationship_.first_previous_relationship_addr != 0)
  {
    struct relationship rel;
    relationship_read(&rel, relationship_.first_previous_relationship_addr);

    if (rel.first_node_addr == relationship_.first_node_addr)
      rel.first_next_relationship_addr = relationship_.first_next_relationship_addr;
    else
      rel.second_next_relationship_addr = relationship_.first_next_relationship_addr;

    relationship_write(&rel, relationship_.first_previous_relationship_addr);
  }
  if (relationship_.second_previous_relationship_addr != 0)
  {
    struct relationship rel;
    relationship_read(&rel, relationship_.second_previous_relationship_addr);

    if (rel.second_node_addr == relationship_.second_node_addr)
      rel.second_next_relationship_addr = relationship_.second_next_relationship_addr;
    else
      rel.first_next_relationship_addr = relationship_.second_next_relationship_addr;

    relationship_write(&rel, relationship_.second_previous_relationship_addr);
  }

  if (relationship_.first_next_relationship_addr != 0)
  {
    struct relationship rel;
    relationship_read(&rel, relationship_.first_next_relationship_addr);

    if (rel.first_node_addr == relationship_.first_node_addr)
      rel.first_previous_relationship_addr = relationship_.first_previous_relationship_addr;
    else
      rel.second_previous_relationship_addr = relationship_.first_previous_relationship_addr;

    relationship_write(&rel, relationship_.first_next_relationship_addr);
  }
  if (relationship_.second_next_relationship_addr != 0)
  {
    struct relationship rel;
    relationship_read(&rel, relationship_.second_next_relationship_addr);

    if (rel.second_node_addr == relationship_.second_node_addr)
      rel.second_previous_relationship_addr = relationship_.second_previous_relationship_addr;
    else
      rel.first_previous_relationship_addr = relationship_.second_previous_relationship_addr;

    relationship_write(&rel, relationship_.second_next_relationship_addr);
  }

  dynamic_store_remove(relationship_.relationship_type_block_addr);

  remove_data(relationship_addr, sizeof(struct relationship));

}
