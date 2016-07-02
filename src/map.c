#define WIN32_LEAN_AND_MEAN
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <zlib.h>
#include "endian.h"
#include "err.h"
#include "map.h"

int mca_open(map_file *file, const char *path) {
  memset(file, 0, sizeof(*file));
  file->file = fopen(path, "r+");
  if (file->file == NULL)
    return MCME_FILE_NOT_FOUND;
  file->chunk_headers = malloc(1024 * sizeof(*file->chunk_headers));
  if (!file->chunk_headers)
    return MCME_OUT_OF_MEMORY;
  uint32_t data[1024];
  int n_chunks = 0;
  size_t n = fread(data, 4, 1024, file->file);
  if (n < 1024) {
    fclose(file->file);
    free(file->chunk_headers);
    return MCME_MALFORMED_MAP;
  }

  for (int i = 0; i < 1024; ++i) {
    if (!data[i])
      continue;
    chunk_header *header = file->chunk_headers + n_chunks;
    header->sector_count = data[i] & 0x000000ff;
    data = swap24(data[i]);
    data >>= 8;
    header->offset = data[i];
    header->header_offset = n_chunks * 4;
    ++n_chunks;
  }

  file->n_chunks = n_chunks;
  uint32_t ts;

  for (int i = 0; i < file->n_chunks;) {
    n = fread(&ts, 4, 1, file->file);
    if (!n || i >= 1024) {
      fclose(file->file);
      free(file->chunk_headers);
      return MCME_MALFORMED_MAP;
    }

    if (!ts)
      continue;

    ts = swap32(ts);
    file->chunk_headers[i++].timestamp = ts;
  }

  return MCME_SUCCESS;
}

void mca_close(map_file *file) {
  free(file->chunk_headers);
  fclose(file->file);
}