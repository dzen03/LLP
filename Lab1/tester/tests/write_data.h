#ifndef LAB1_LAB1_TESTER_TESTS_WRITE_DATA_H_
#define LAB1_LAB1_TESTER_TESTS_WRITE_DATA_H_

#include "backend.h"
#include "dynamic_store.h"
#include "file.h"
#include "graph.h"
#include "node.h"
#include "property.h"
#include "relationship.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE1 STRING
#define TYPE2 INT
#define KEY1 "Key1"
#define KEY2 "Key2"
#define VALUE1 "Value1"
#define VALUE2 0x12345

void write(char* filename)
{
  fclose(fopen(filename, "w")); // clear file

  backend_start(filename);
  char *key1_ = KEY1;
  char *key2_ = KEY2;
  char *value1_ = VALUE1;

  struct dynamic_store key1 = {{.length=strlen(key1_) + 1}, (uint8_t *) key1_};

  struct dynamic_store value1 = {{.length=strlen(value1_) + 1}, (uint8_t *) value1_};

  int64_t key1_addr = dynamic_store_write(&key1, 0);
  assert(key1_addr >= 0);
  int64_t value1_addr = dynamic_store_write(&value1, 0);
  assert(value1_addr >= 0);

  struct property property1 = {TYPE1, key1_addr, {.addr = value1_addr}, 0};

  struct dynamic_store key2 = {{.length=strlen(key2_) + 1}, (uint8_t *) key2_};

  int64_t key2_addr = dynamic_store_write(&key2, 0);
  assert(key2_addr >= 0);

  struct property property2 = {TYPE2, key2_addr, {VALUE2}, 0};

  assert(property_write(&property1, 0) >= 0);
//    property2.previous_property_addr = property1_addr;
  int64_t property2_addr = property_write(&property2, 0);
  assert(property2_addr >= 0);

  property1.next_property_addr = property2_addr;
  int64_t property1_addr = property_write(&property1, 0);
  assert(property1_addr >= 0);

  struct node first_node = (struct node) {0, property1_addr};
  struct node second_node = (struct node) {0, 0};

  int64_t first_node_addr = node_write(&first_node, 0);
  assert(first_node_addr >= 0);
  int64_t second_node_addr = node_write(&second_node, 0);
  assert(first_node_addr >= 0);

  char *relationship_name1_ = "Relationship1";
  struct dynamic_store relationship_name1 = {{.length=strlen(relationship_name1_) + 1}, (uint8_t *) relationship_name1_};
  int64_t relationship_name1_addr = dynamic_store_write(&relationship_name1, 0);
  assert(relationship_name1_addr >= 0);

  struct relationship relationship = {BIDIRECTIONAL, first_node_addr, second_node_addr, relationship_name1_addr};

  int64_t relationship1 = relationship_write(&relationship, 0);
  assert(relationship1 >= 0);


  assert(node_read(&first_node, first_node_addr) == 0);
  assert(node_read(&second_node, second_node_addr) == 0);
  first_node.next_relationship_addr = relationship1;
  second_node.next_relationship_addr = relationship1;

  assert(node_write(&first_node, first_node_addr) == first_node_addr);
  assert(node_write(&second_node, second_node_addr) == second_node_addr);

  backend_stop();
}
#endif //LAB1_LAB1_TESTER_TESTS_WRITE_DATA_H_
