#pragma once
#include <stdio.h>
typedef struct chunk_header {
  int offset;
  int sector_count;
  int header_offset;
  int timestamp;
} chunk_header;

typedef struct map_file {
  FILE *file;
  chunk_header *chunk_headers;
  int n_chunks;
  // chunk_data *chunk_data;
} map_file;

int mca_open(map_file *file, const char *path);
void mca_close(map_file *file);

int map_read();