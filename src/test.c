#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "slist.h"
#include "nbt.h"

int main(int argc, char **argv) {  
  slist list;
  printf("Stationary list code: %d\n", slist_init(&list));
  for (int i = 0; i < 100; ++i) {
    int *ki = malloc(sizeof(*ki));
    *ki = i;
    slist_push(&list, ki);
  }

  slist_node *curr = list.start;
  for (int i = 0; i < 100; ++i) {
    printf("%d ", *((int *) curr->data));
    curr = curr->next;
  }

  slist_destroy(&list);
}