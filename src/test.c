#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "map.h"
#include "slist.h"
#include "nbt.h"

int main(int argc, char **argv) {
  map_file file;
  printf("mca_open code: %d\n", mca_open(&file, "r.0.0.mca"));
  printf("# of chunks: %d\n", file.n_chunks);
  printf("First chunk timestamp: %d\n", file.chunk_headers->timestamp);
  printf("First chunk sector count: %d\n", file.chunk_headers->sector_count);
  printf("First chunk header offset: %d\n", file.chunk_headers->header_offset);
  printf("First chunk data offset: %d\n", file.chunk_headers->offset);

  slist list;
  printf("Stationary list code: %d\n", slist_init(&list));
  for (int i = 0; i < 100; ++i) {
    int *ki = malloc(sizeof(*ki));
    *ki = i;
    slist_push_back(&list, ki);
  }

  slist_node *curr = list.start;
  for (int i = 0; i < 100; ++i) {
    printf("%d ", *((int *) curr->data));
    curr = curr->next;
  }

  slist_destroy(&list);
}