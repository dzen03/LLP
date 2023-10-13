#ifndef LAB1_LAB1_SOLUTION_SRC_BACKEND_H_
#define LAB1_LAB1_SOLUTION_SRC_BACKEND_H_

#include "dynamic_store.h"
#include "node.h"
#include "property.h"
#include "relationship.h"

struct file get_file(void);

void backend_start(char* filename);
void backend_stop(void);

// if addr == 0, it will be placed somewhere
int64_t node_write(struct node* data, int64_t addr);
// if addr == 0, it will be placed somewhere
int64_t relationship_write(struct relationship* data, int64_t addr);
// if addr == 0, it will be placed somewhere
int64_t property_write(struct property* data, int64_t addr);

// if addr == 0, it will be placed somewhere
int64_t dynamic_store_write(const struct dynamic_store* data, int64_t addr);
// if addr == 0, it will be placed somewhere
int64_t dynamic_store_write_and_free(struct dynamic_store* data, int64_t addr);


int node_read(struct node* node, int64_t addr);
int relationship_read(struct relationship* relationship, int64_t addr);
int property_read(struct property* property, int64_t addr);

int dynamic_store_read(struct dynamic_store* store, int64_t addr);

#endif //LAB1_LAB1_SOLUTION_SRC_BACKEND_H_
