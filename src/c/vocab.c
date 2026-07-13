#include <pebble.h>
#include <stdlib.h>
#include "vocab.h"

// Binary layout of vocab_db.bin (little-endian), see tools/generate_vocab.py:
//   header:  "VDB2" (4) | count uint16 | max_record uint16
//   offsets: (count+1) x uint32  (relative to data section)
//   diffs:   count x uint8
//   data:    per record, 6 NUL-terminated strings:
//            term, meaning, example, etymology, phonetic, category

#define HEADER_SIZE 8
#define RECORD_BUF_SIZE 512

static ResHandle s_handle;
static uint16_t s_count = 0;
static uint32_t *s_offsets = NULL;  // count+1 entries (heap)
static uint8_t *s_diffs = NULL;     // count entries (heap)
static uint32_t s_data_base = 0;

static char s_record_buf[RECORD_BUF_SIZE];
static VocabEntry s_entry;
static int s_loaded_index = -1;

bool vocab_init(void) {
  s_handle = resource_get_handle(RESOURCE_ID_VOCAB_DB);
  if (!s_handle) return false;

  uint8_t header[HEADER_SIZE];
  if (resource_load_byte_range(s_handle, 0, header, HEADER_SIZE) != HEADER_SIZE) {
    return false;
  }
  if (memcmp(header, "VDB2", 4) != 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "vocab_db.bin: bad magic");
    return false;
  }
  uint16_t count;
  memcpy(&count, header + 4, 2);
  if (count == 0) return false;
  if (count > MAX_VOCAB) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "vocab_db.bin has %d words, capping at %d",
            count, MAX_VOCAB);
    count = MAX_VOCAB;
  }

  s_offsets = malloc((count + 1) * sizeof(uint32_t));
  s_diffs = malloc(count);
  if (!s_offsets || !s_diffs) {
    vocab_deinit();
    return false;
  }

  // Offsets and difficulty positions are laid out for the count stored in
  // the file, which may exceed our capped count; compute from the header.
  uint16_t file_count;
  memcpy(&file_count, header + 4, 2);
  uint32_t offsets_pos = HEADER_SIZE;
  uint32_t diffs_pos = offsets_pos + (file_count + 1) * sizeof(uint32_t);
  s_data_base = diffs_pos + file_count;

  if (resource_load_byte_range(s_handle, offsets_pos, (uint8_t *)s_offsets,
                               (count + 1) * sizeof(uint32_t)) !=
      (int)((count + 1) * sizeof(uint32_t))) {
    vocab_deinit();
    return false;
  }
  if (resource_load_byte_range(s_handle, diffs_pos, s_diffs, count) != count) {
    vocab_deinit();
    return false;
  }

  s_count = count;
  s_loaded_index = -1;
  return true;
}

void vocab_deinit(void) {
  free(s_offsets);
  s_offsets = NULL;
  free(s_diffs);
  s_diffs = NULL;
  s_count = 0;
  s_loaded_index = -1;
}

int vocab_count(void) { return s_count; }

int vocab_difficulty(int index) {
  if (index < 0 || index >= s_count) return 0;
  return s_diffs[index];
}

const VocabEntry* vocab_get(int index) {
  if (index < 0 || index >= s_count) return NULL;
  if (index == s_loaded_index) return &s_entry;

  uint32_t start = s_offsets[index];
  uint32_t len = s_offsets[index + 1] - start;
  if (len == 0 || len >= RECORD_BUF_SIZE) return NULL;

  if (resource_load_byte_range(s_handle, s_data_base + start,
                               (uint8_t *)s_record_buf, len) != (int)len) {
    return NULL;
  }
  s_record_buf[len] = '\0';  // safety terminator

  // Walk the 6 NUL-terminated strings.
  const char *fields[6];
  const char *p = s_record_buf;
  const char *end = s_record_buf + len;
  for (int i = 0; i < 6; i++) {
    if (p >= end) return NULL;
    fields[i] = p;
    p += strlen(p) + 1;
  }

  s_entry = (VocabEntry) {
    .term = fields[0],
    .meaning = fields[1],
    .example = fields[2],
    .etymology = fields[3],
    .phonetic = fields[4],
    .category = fields[5],
    .difficulty = s_diffs[index],
  };
  s_loaded_index = index;
  return &s_entry;
}
