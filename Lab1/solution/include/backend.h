#ifndef LAB1_LAB1_SOLUTION_SRC_BACKEND_H_
#define LAB1_LAB1_SOLUTION_SRC_BACKEND_H_

#include "dynamic_store.h"
#include "file.h"
#include "free_space.h"
#include "node.h"
#include "property.h"
#include "relationship.h"

#define STATIC_STORE_SIZE 58

int update_metadata(struct file file);

struct file get_file(void);

void backend_start(char* filename);
void backend_stop(void);

// if addr == 0, it will be placed somewhere
int64_t node_write(struct node* data, int64_t addr);
// if addr == 0, it will be placed somewhere
int64_t relationship_write(struct relationship* data, int64_t addr);
// if addr == 0, it will be placed somewhere
int64_t property_write(struct property* data, int64_t addr);

// need addr
int64_t free_space_write(struct free_space* data, int64_t addr, enum free_space_type type);

// if addr == 0, it will be placed somewhere
// deprecated
int64_t dynamic_store_write(const struct dynamic_store* data, int64_t addr);

int64_t dynamic_store_write_chain(uint8_t* data, uint64_t length, int64_t addr);

// allocates memory inside!
uint64_t dynamic_store_read_chain(uint8_t** data, int64_t addr);

// if addr == 0, it will be placed somewhere
// deprecated + unused
int64_t dynamic_store_write_and_free(struct dynamic_store* data, int64_t addr);


int node_read(struct node* node, int64_t addr);
int relationship_read(struct relationship* relationship, int64_t addr);
int property_read(struct property* property, int64_t addr);
int free_space_read(struct free_space* free_space, int64_t addr);

int dynamic_store_read(struct dynamic_store* store, int64_t addr);

void dynamic_store_remove(int64_t addr);

int remove_data(int64_t addr, int64_t size);

#endif //LAB1_LAB1_SOLUTION_SRC_BACKEND_H_
