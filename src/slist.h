#pragma once
// stationary list
typedef struct slist_node {
  void *data;
  struct slist_node *next;
  struct slist_node *prev;
} slist_node;

typedef struct slist {
  size_t size;
  slist_node *start;
  slist_node *last;
} slist;

int slist_init(slist *list);
int slist_push(slist *list, void *data);
void *slist_pop(slist *list);
void *slist_top(slist *list);
void slist_destroy(slist *list);