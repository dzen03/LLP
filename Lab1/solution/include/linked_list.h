#ifndef LAB1_SOLUTION_SRC_LINKED_LIST_H_
#define LAB1_SOLUTION_SRC_LINKED_LIST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define linked_list_def(type) \
struct linked_list            \
{                             \
  type value;                 \
  struct linked_list* next;   \
};

#define linked_list_find_def(type, comp)                                    \
struct linked_list* linked_list_find(struct linked_list* root, type value)  \
{                                                                           \
  struct linked_list* node = root;                                          \
  while (node != NULL)                                                      \
  {                                                                         \
    if (comp(node->value, value))                                           \
      return node;                                                          \
    node = node->next;                                                      \
  }                                                                         \
  return NULL;                                                              \
}

#define linked_list_push_back_def(type)                           \
void linked_list_push_back(struct linked_list** root, type value) \
{                                                                 \
  if (*root == NULL)                                              \
  {                                                               \
    *root = malloc(sizeof(**root));                               \
    (*root)->value = value;                                       \
    (*root)->next = NULL;                                         \
    return;                                                       \
  }                                                               \
                                                                  \
  struct linked_list* node = *root;                               \
  while (node->next != NULL)                                      \
    node = node->next;                                            \
                                                                  \
  node->next = malloc(sizeof(**root));                            \
  node->next->value = value;                                      \
  node->next->next = NULL;                                        \
}

#define linked_list_free_def                    \
void linked_list_free(struct linked_list* root) \
{                                               \
  struct linked_list* node = root;              \
  struct linked_list* prev_node = root;         \
  while(node != NULL)                           \
  {                                             \
    prev_node = node;                           \
    node = node->next;                          \
    free(prev_node);                            \
  }                                             \
}

#endif //LAB1_SOLUTION_SRC_LINKED_LIST_H_
