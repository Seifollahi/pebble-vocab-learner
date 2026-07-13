#pragma once
#include <pebble.h>

// Runtime accessor for the vocabulary database resource (vocab_db.bin,
// generated from resources/vocab_db.json by tools/generate_vocab.py).
// Words live in the 256KB resource bank, not in app RAM; records are read
// on demand into a single static buffer.

// Compile-time ceiling used to size SRS state arrays. The packed persist
// format costs 2 bytes/word, so ~1900 words also fit the 4KB persist quota.
#ifdef PBL_PLATFORM_APLITE
#define MAX_VOCAB 400
#else
#define MAX_VOCAB 1800
#endif

typedef struct {
  const char *term;
  const char *meaning;
  const char *example;
  const char *etymology;
  const char *phonetic;
  const char *category;
  int difficulty;  // 1=basic, 2=intermediate, 3=advanced
} VocabEntry;

// Loads the header, offset index and difficulty table. Call once at startup,
// before state_load_config(). Returns false if the resource is missing or
// malformed (vocab_count() will be 0).
bool vocab_init(void);
void vocab_deinit(void);

int vocab_count(void);
int vocab_difficulty(int index);

// Loads word `index` from the resource into a static buffer and returns a
// pointer to it. The returned entry (and all its strings) is only valid
// until the next vocab_get() call. Returns NULL on bad index or read error.
const VocabEntry* vocab_get(int index);
