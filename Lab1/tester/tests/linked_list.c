#include "../../solution/include/linked_list.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

bool cmp(int a, int b)
{
  return a == b;
}

linked_list_def(int)
linked_list_find_def(int, cmp)
linked_list_push_back_def(int)
linked_list_free_def

int main(void)
{
  struct linked_list* ll = NULL;

  linked_list_push_back(&ll, 1);
  linked_list_push_back(&ll, 2);
  linked_list_push_back(&ll, 3);
  linked_list_push_back(&ll, 4);
  linked_list_push_back(&ll, 5);


  struct linked_list* ll2 = linked_list_find(ll, 3);
  struct linked_list* ll3 = linked_list_find(ll, -1);

  assert(ll->next->next == ll2);
  assert(ll3 == NULL);

  linked_list_free(ll);
  return 0;
}
