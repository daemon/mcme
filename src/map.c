#include <stdlib.h>
#include <stdint.h>
#include "endian.h"
#include "err.h"
#include "map.h"

int mca_open(map_file *file, const char *path) {
  file->file = fopen(path, "r+");
  if (file->file == NULL)
    return MCME_FILE_NOT_FOUND;
  file->chunk_headers = malloc(1024 * sizeof(*file->chunk_headers));
  if (!file->chunk_headers)
    return MCME_OUT_OF_MEMORY;
  uint32_t data;
  size_t n = 0;
  int n_chunks = 0;
  for (int i = 0; i < 1024; ++i) {
    n = fread(&data, 4, 1, file->file);
    if (!n) {
      fclose(file->file);
      free(file->chunk_headers);
      return MCME_MALFORMED_MAP;
    }

    if (!data)
      continue;

    chunk_header *header = file->chunk_headers + n_chunks;
    header->sector_count = data & 0x000000ff;
    swap24(data);
    data >>= 8;
    header->offset = data;
    header->header_offset = n_chunks * 4;
    ++n_chunks;
  }

  file->n_chunks = n_chunks;

  for (int i = 0; i < file->n_chunks;) {
    n = fread(&data, 4, 1, file->file);
    if (!n || i >= 1024) {
      fclose(file->file);
      free(file->chunk_headers);
      return MCME_MALFORMED_MAP;
    }

    if (!data)
      continue;

    swap32(data);
    file->chunk_headers[i++].timestamp = data;
  }

  return MCME_SUCCESS;
}

void mca_close(map_file *file) {
  free(file->chunk_headers);
  fclose(file->file);
}