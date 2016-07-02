#pragma once
#include <stdint.h>
#include <wchar.h>
#include "err.h"
#include "slist.h"

#define NBT_TAG_END         0
#define NBT_TAG_BYTE        1
#define NBT_TAG_SHORT       2
#define NBT_TAG_INT         3
#define NBT_TAG_LONG        4
#define NBT_TAG_FLOAT       5
#define NBT_TAG_DOUBLE      6
#define NBT_TAG_BYTE_ARRAY  7
#define NBT_TAG_STRING      8
#define NBT_TAG_LIST        9
#define NBT_TAG_COMPOUND    10
#define NBT_TAG_INT_ARRAY   11

typedef struct nbt_header {
  uint8_t id;
  int16_t name_length;
  wchar_t *name;
} nbt_header;

typedef struct nbt_byte_arr {
  int32_t length;
  uint8_t *data;
} nbt_byte_arr;

typedef struct nbt_string {
  int16_t length;
  wchar_t *string;
} nbt_string;

typedef struct nbt_list {
  uint8_t id;
  slist list;
} nbt_list;

typedef struct nbt_int_arr {
  int32_t length;
  int32_t *data;
} nbt_int_arr;

typedef struct nbt_tag {
  nbt_header header;
  union {
    int8_t data_byte;
    int16_t data_short;
    int32_t data_int;
    int64_t data_long;
    float data_float;
    double data_double;
    nbt_byte_arr data_byte_arr;
    nbt_string data_string;
    nbt_list data_list;
    slist data_compound;
    nbt_int_arr data_int_arr;
  };
} nbt_tag;

typedef enum reader_state reader_state;

typedef struct nbt_reader {
  char buffer[4096];
  int buf_size;
  slist tag_stack;
  reader_state state;
} nbt_reader;

int nbt_parse(MCME_OUT nbt_tag *tag, char *buf, size_t size);
int nbt_read_file(MCME_OUT nbt_tag *tag, const char *name);
int nbt_reader_init(nbt_reader *reader);
int nbt_reader_put(nbt_reader *reader, char *data, size_t size);
int nbt_reader_finish(nbt_reader *reader, MCME_OUT nbt_tag *tag);
void nbt_reader_destroy(nbt_reader *reader);
void nbt_tag_destroy(nbt_tag *tag);