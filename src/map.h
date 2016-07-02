#pragma once
#include <stdint.h>
#include <stdio.h>
typedef struct chunk_header {
  int offset;
  int sector_count;
  int header_offset;
  int timestamp;
} chunk_header;

typedef struct chunk_data {
  int32_t x, z;
  int8_t *block_ids;
  int8_t *block_data;
  int64_t last_update;
  uint8_t light_populated;
  uint8_t terrain_populated;
  int64_t inhabited_time;
  uint8_t *biomes;
  int32_t *height_map;
} chunk_data;

typedef struct map_file {
  FILE *file;
  chunk_header *chunk_headers;
  chunk_data *chunk_data;
  int n_chunks;
  // HANDLE win32_file;
} map_file;

int mca_open(map_file *file, const char *path);
void mca_close(map_file *file);

int mca_read(map_file *file);