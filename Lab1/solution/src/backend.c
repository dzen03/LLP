#include "backend.h"
#include "file.h"
#include "utils.h"

#include "dynamic_store.h"
#include "node.h"
#include "property.h"
#include "relationship.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct file file_;

struct file get_file(void)
{
  return file_;
}

void backend_start(char* filename)
{
  file_ = file_init(filename);

  if (file_.descriptor == NULL)
    exit_with_error("Can't open file. Check your filename");

}

int64_t get_available_space(__attribute__((unused)) int64_t size) // TODO rewrite in prod using max_heap, for testing only
{
  return fsizeo(file_.descriptor);
}

int update_metadata(void)
{
  return static_store_write(file_.descriptor, 0, &file_.metadata, sizeof(file_.metadata));
}

// invalidates previous node's local variable
int64_t node_write(struct node* data, int64_t addr)
{
  int insert = 0;
  if (addr == 0)
  {
    insert = 1;
    addr = get_available_space(sizeof(struct node));
  }

  if (insert)
  {
    if (file_.metadata.last_node_addr != 0)
    {
      struct node node;
      node_read(&node, file_.metadata.last_node_addr);
      node.next_node_addr = addr;
      static_store_write(file_.descriptor, file_.metadata.last_node_addr, &node, sizeof(struct node));
    }
    data->previous_node_addr = file_.metadata.last_node_addr;
    file_.metadata.last_node_addr = addr;
  }

  int res;
  if ((res = static_store_write(file_.descriptor, addr, data, sizeof(struct node))) != 0)
    return res;


  if (file_.metadata.first_node_addr == 0)
  {
    insert = 1;
    file_.metadata.first_node_addr = addr;
  }

  if (insert)
    res = update_metadata();

  return (res == 0 ? addr : res);
}

int64_t relationship_write(struct relationship* data, int64_t addr)
{
  if (addr == 0)
    addr = get_available_space(sizeof(struct node));

  int res;
  if ((res = static_store_write(file_.descriptor, addr, data, sizeof(struct relationship))) != 0)
    return res;

  return (res == 0 ? addr : res);
}

int64_t property_write(struct property* data, int64_t addr)
{
  if (addr == 0)
    addr = get_available_space(sizeof(struct property));
  int res = static_store_write(file_.descriptor, addr, data, sizeof(struct property));
  return (res == 0 ? addr : res);
}

int64_t dynamic_store_write(const struct dynamic_store* const data, int64_t addr)
{
  if (addr == 0)
    addr = get_available_space(sizeof(struct relationship));
  int res = dynamic_store_write_(file_.descriptor, addr, data);
  return (res == 0 ? addr : res);
}

int64_t dynamic_store_write_and_free(struct dynamic_store* data, int64_t addr)
{
  if (addr == 0)
    addr = get_available_space(sizeof(struct relationship));
  int res = dynamic_store_write_and_free_(file_.descriptor, addr, data);
  return (res == 0 ? addr : res);
}

int node_read(struct node* node, int64_t addr)
{
  return static_store_read(file_.descriptor, addr, node, sizeof(struct node));
}

int relationship_read(struct relationship* relationship, int64_t addr)
{
  return static_store_read(file_.descriptor, addr, relationship, sizeof(struct relationship));
}
int property_read(struct property* property, int64_t addr)
{
  return static_store_read(file_.descriptor, addr, property, sizeof(struct property));
}

// Inside allocates memory for data; you should free it!
int dynamic_store_read(struct dynamic_store* store, int64_t addr)
{
  return dynamic_store_read_(file_.descriptor, addr, store);
}

void backend_stop(void)
{
  file_close(file_.descriptor);
}
