#include "backend.h"
#include "file.h"
#include "utils.h"

#include "dynamic_store.h"
#include "free_space.h"
#include "node.h"
#include "property.h"
#include "relationship.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int update_metadata(void);

struct file file_;

struct file get_file(void)
{
  return file_;
}

void check_all_requirements(void)
{
  static_assert(sizeof(struct node) == STATIC_STORE_SIZE, "node is not padded");
  static_assert(sizeof(struct property) == STATIC_STORE_SIZE, "property is not padded");
  static_assert(sizeof(struct relationship) == STATIC_STORE_SIZE, "relationship is not padded");
}

void backend_start(char* filename)
{
  check_all_requirements();

  file_ = file_init(filename);

  if (file_.descriptor == NULL)
    exit_with_error("Can't open file. Check your filename");

}

int64_t get_available_space(int64_t size)
{
  int64_t addr;
  struct free_space space;

  if (size == DYNAMIC_STORE_SIZE)
  {
    if (file_.metadata.first_free_dynamic_addr == 0)
    {
      addr = fsizeo(file_.descriptor);
    }
    else
    {
      addr = file_.metadata.first_free_dynamic_addr;
      free_space_read(&space, file_.metadata.first_free_dynamic_addr);
      if (file_.metadata.first_free_dynamic_addr == file_.metadata.last_free_dynamic_addr)
        file_.metadata.first_free_dynamic_addr = file_.metadata.last_free_dynamic_addr = 0;
      else
        file_.metadata.first_free_dynamic_addr = space.next_addr;
      update_metadata();
    }
  }
  else if (size == STATIC_STORE_SIZE)
  {
    if (file_.metadata.first_free_static_addr == 0)
    {
      addr = fsizeo(file_.descriptor);
    }
    else
    {
      addr = file_.metadata.first_free_static_addr;
      free_space_read(&space, file_.metadata.first_free_static_addr);
      if (file_.metadata.first_free_static_addr == file_.metadata.last_free_static_addr)
        file_.metadata.first_free_static_addr = file_.metadata.last_free_static_addr = 0;
      else
        file_.metadata.first_free_static_addr = space.next_addr;
    }
  }
  else
  {
    exit_with_error("You are trying to write structure of unknown size");
    return 0;
  }
  return addr;
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

int64_t free_space_write(struct free_space* data, int64_t addr, enum free_space_type type)
{
  if (addr < 0)
    exit_with_error("Incorrect address");

  int64_t* first_free_addr_p;
  int64_t* last_free_addr_p;

  if (type == DYNAMIC)
  {
    first_free_addr_p = &file_.metadata.first_free_dynamic_addr;
    last_free_addr_p = &file_.metadata.last_free_dynamic_addr;
  }
  else if (type == STATIC)
  {
    first_free_addr_p = &file_.metadata.first_free_static_addr;
    last_free_addr_p = &file_.metadata.last_free_static_addr;
  }
  else
  {
    exit_with_error("There is no such type");
    return -1;
  }

  if (file_.metadata.last_free_dynamic_addr != 0)
  {
    struct free_space free_space;
    free_space_read(&free_space, *last_free_addr_p);
    free_space.next_addr = addr;
    static_store_write(file_.descriptor, *last_free_addr_p,&free_space, sizeof(struct free_space));
  }
//    data->previous_node_addr = file_.metadata.last_node_addr;
  *last_free_addr_p = addr;

  int res;
  if ((res = static_store_write(file_.descriptor, addr, data, sizeof(struct free_space))) != 0)
    return res;


  if (*first_free_addr_p == 0)
    *first_free_addr_p = addr;

  res = update_metadata();

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

int free_space_read(struct free_space* free_space, int64_t addr)
{
  return static_store_read(file_.descriptor, addr, free_space, sizeof(struct free_space));
}

// Inside allocates memory for data; you should free it!
// TODO consider switching to preallocated memory instead
int dynamic_store_read(struct dynamic_store* store, int64_t addr)
{
  return dynamic_store_read_(file_.descriptor, addr, store);
}



void backend_stop(void)
{
  file_close(file_.descriptor);
}
