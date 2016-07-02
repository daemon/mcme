#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <zlib.h>
#include "endian.h"
#include "err.h"
#include "nbt.h"
#include "slist.h"

int nbt_parse(MCME_OUT nbt_tag *tag, char *buf, size_t size) {
  size_t cursor = 0;
  nbt_header *header = &tag->header;
  header->id = (uint8_t) buf[cursor];
  if (header->id == NBT_TAG_END)
    (void) tag; // do nothing
  ++cursor;
  header->name_length = swap16(*((int16_t *) buf + cursor));
  wchar_t *name = malloc(header->name_length * sizeof(*name));
  ++cursor;
  mbstate_t mbs1;
  memset(&mbs1, 0, sizeof(mbs1));
  mbrtowc(name, buf + cursor, header->name_length, &mbs1);
  wprintf(name);
  return MCME_SUCCESS;
}

int nbt_read_file(nbt_tag *tag, const char *name) {
  z_stream zs;
  zs.zalloc = Z_NULL;
  zs.zfree = Z_NULL;
  zs.opaque = Z_NULL;
  int err = inflateInit(&zs);
  switch (err) {
    case Z_OK:
    break;
    case Z_MEM_ERROR:
    return MCME_OUT_OF_MEMORY;
    default:
    return MCME_FAILURE;
  }

  FILE *file = fopen(name, "r");
  if (!file) {
    fclose(file);
    inflateEnd(&zs);
    return MCME_FILE_NOT_FOUND;
  }

  char buf[2048];
  char chunk[2048];
  size_t n = 0;
  int rc;
  int ready = 0;
  nbt_reader reader;
  nbt_reader_init(&reader);
  do {
    n = fread(buf, 1, 2048, file);
    if (!n) {
      inflateEnd(&zs);
      fclose(file);
      return MCME_ILLFORMED_NBT;
    }

    do {
      zs.avail_out = 2048;
      zs.next_out = chunk;
      rc = inflate(&zs, Z_NO_FLUSH);
      switch (rc) {
        case Z_NEED_DICT:
        rc = Z_DATA_ERROR;
        case Z_DATA_ERROR:
          inflateEnd(&zs);
          fclose(file);
          return MCME_ILLFORMED_NBT;
        case Z_MEM_ERROR:
          inflateEnd(&zs);
          fclose(file);
          return MCME_OUT_OF_MEMORY;
      }

      ready = 2048 - zs.avail_out;
      nbt_reader_put(&reader, chunk, ready);
    } while (!zs.avail_out);
  } while (rc != Z_STREAM_END);
  fclose(file);
  inflateEnd(&zs);
  rc = nbt_reader_finish(&reader, tag);
  nbt_reader_destroy(&reader);
  return rc;
}

/*** NBT reader ******************************************/
typedef enum reader_state {
  /* constructing tags */
  CONSTRUCT_NEW = 10000,
  /* finished */
  FINISHED,
  /* expecting data */
  ACCEPT_BYTE = 1,
  ACCEPT_SHORT,
  ACCEPT_INT,
  ACCEPT_LONG,
  ACCEPT_FLOAT,
  ACCEPT_DOUBLE,
  ACCEPT_BYTE_ARRAY,
  ACCEPT_STRING,
  ACCEPT_LIST,
  ACCEPT_COMPOUND,
  ACCEPT_INT_ARRAY
} reader_state;

int nbt_reader_init(nbt_reader *reader) {
  reader->state = CONSTRUCT_NEW;
  reader->buf_size = 0;
  return slist_init(&reader->tag_stack);
}

static inline int do_cons_new(nbt_reader *reader, size_t *size, char **read_ptr) {
  uint8_t id = **read_ptr;
  if (id > 11)
    return MCME_ILLFORMED_NBT;
  // still need to handle TAG_END or null TAG_BYTE
  --*size;
  ++*read_ptr;
  if (*size < 2) {
    ++*size;
    --*read_ptr;
    return MCME_INPUT_REQUIRED;
  }
  int16_t namelen = swap16(*((int16_t *) (*read_ptr)));
  *size -= 2;
  *read_ptr += 2;
  if (*size < namelen) {
    *size += 3;
    *read_ptr -= 3;
    return MCME_INPUT_REQUIRED;
  }

  reader->state = id;
  nbt_tag *tag = malloc(sizeof(*tag));
  if (!tag)
    return MCME_OUT_OF_MEMORY;
  nbt_header *header = &tag->header;
  header->id = id;
  header->name_length = namelen;
  wchar_t *name = malloc(header->name_length * sizeof(*name) + 1);
  if (!name)
    return MCME_OUT_OF_MEMORY;
  mbstate_t mbs1;
  memset(&mbs1, 0, sizeof(mbs1));
  mbrtowc(name, read_ptr, header->name_length, &mbs1);
  wprintf(name);
  header->name = name;
  return slist_push(&reader->tag_stack, tag);
}

static inline int do_accept(nbt_reader *reader, size_t *size, char **read_ptr) {
  nbt_tag *tag = slist_top(&reader->tag_stack);
  if (!tag)
    return MCME_ILLFORMED_NBT;
  int shift = 0;
  switch (reader->state) {
  case ACCEPT_BYTE:
    tag->data_byte = **read_ptr;
    shift = 1;
    break;
  case ACCEPT_SHORT:
    tag->data_short = swap16(*((int16_t *) *read_ptr));
    shift = 2;
    break;
  case ACCEPT_INT:
    tag->data_int = swap32(*((int32_t *) *read_ptr));
    shift = 4;
    break;
  case ACCEPT_LONG:
    tag->data_long = swap64(*((int64_t *) *read_ptr));
    shift = 8;
    break;
  case ACCEPT_FLOAT:
    int float_int = swap32(*((int32_t *) *read_ptr));
    tag->data_float = *((float *) &float_int);
    shift = 4;
    break;
  case ACCEPT_DOUBLE:
    int double_int = swap64(*((int64_t *) *read_ptr));
    tag->data_double = *((double *) &double_int);
    shift = 8;
    break;
  }

  if (shift > *size)
    return MCME_INPUT_REQUIRED;
  *read_ptr += shift;
  *size -= shift;
  return MCME_SUCCESS;
}

int nbt_reader_put(nbt_reader *reader, char *data, size_t size) {
  if (reader->state == FINISHED)
    return MCME_FINISHED_NBT;
  if (size > 4096 - reader->buf_size)
    return MCME_ILLFORMED_NBT;
  memcpy(reader->buffer + reader->buf_size, data, size);
  size += reader->buf_size + size;
  reader->buf_size = size;
  char *read_ptr = reader->buffer;
  int rc;
  while (size > 0) {
    if (reader->state == CONSTRUCT_NEW)
      rc = do_cons_new(reader, &size, &read_ptr);
    else
      rc = do_accept(reader, &size, &read_ptr);
    switch (rc) {
    case MCME_SUCCESS:
      break;
    case MCME_OUT_OF_MEMORY:
    case MCME_ILLFORMED_NBT:
    case MCME_FAILURE:
      return rc;
    case MCME_INPUT_REQUIRED:
      memcpy(reader->buffer, read_ptr, size);
      return MCME_SUCCESS;
    }
  }

  return MCME_SUCCESS;
}

int nbt_reader_finish(nbt_reader *reader, nbt_tag *tag) {
  reader->state = FINISHED;
  
}

void nbt_reader_destroy(nbt_reader *reader) {
  slist_node *node = reader->tag_stack.start;
  while (node) {
    nbt_tag_destroy(node->data);
    node = node->next;
  }
  slist_destroy(&reader->tag_stack);
}