#include <stdlib.h>
#include "err.h"
#include "slist.h"

int slist_init(slist *list) {
  list->size = 0;
  list->start = malloc(4 * sizeof(*list->start));
  if (!list->start)
    return MCME_OUT_OF_MEMORY;
  return MCME_SUCCESS;
}

int slist_push_back(slist *list, void *data) {
  if (!list->size) {
    list->start->data = data;
    list->start->next = NULL;
    list->start->prev = NULL;
    list->last = list->start;
    list->size = 1;
    return MCME_SUCCESS;
  } else if (list->size >= 4 && !(list->size & (list->size - 1))) {
    list->last->next = malloc(list->size * sizeof(*list->last->next));
    if (!list->last->next)
      return MCME_OUT_OF_MEMORY;
  }

  if (!list->last->next)
    list->last->next = list->last + 1;
  list->last->next->data = data;
  list->last->next->prev = list->last;
  list->last->next->next = NULL;
  list->last = list->last->next;
  ++list->size;
  return MCME_SUCCESS;
}

void slist_destroy(slist *list) {
  slist_node *curr = list->last;
  --list->size;
  while (1) {
    curr = curr->prev;
    if (!(list->size & (list->size - 1)) && list->size > 4)
      free(curr->next);
    else if (list->size <= 4) {
      free(list->start);
      return;
    }

    --list->size;
  }
}