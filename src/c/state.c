#include "state.h"
#include "vocab.h"
#include "ui.h"

// --- Persist keys -----------------------------------------------------------
#define PERSIST_NOTIF_ENABLED 1
#define PERSIST_NOTIF_FREQ 2
#define PERSIST_VIBRATION 3
#define PERSIST_DIFF_BASIC 4
#define PERSIST_DIFF_INTERMEDIATE 5
#define PERSIST_DIFF_ADVANCED 6
#define PERSIST_WORDS_LEARNED 7
#define PERSIST_WORDS_REVIEWED 8
#define PERSIST_LAUNCH_COUNT 9

#define PERSIST_SCHEMA_VERSION 99
// v1 (legacy) stored the whole arrays as single blobs at these keys:
#define PERSIST_V1_BUCKETS 100
#define PERSIST_V1_TIMES 101
// v2 stores chunked data. Pebble limits each persist value to 256 bytes,
// so arrays are split across consecutive keys.
#define PERSIST_BUCKETS_BASE 110
#define PERSIST_TIMES_BASE 130

#define SCHEMA_VERSION 2
#define CHUNK_BYTES 248
#define V1_VOCAB_COUNT 20

// --- Session state -----------------------------------------------------------
static int s_current_vocab_index = 0;
static int s_minutes_remaining = 10; // auto-advance after 10 idle minutes
static bool s_meaning_revealed = false;
static int s_display_mode = 0;
static int s_words_learned = 0;
static int s_words_reviewed = 0;
static int s_launch_count = 0;
static bool s_all_caught_up = false;
static time_t s_next_due_time = 0;

static uint8_t s_vocab_buckets[VOCAB_COUNT] = {0};
static int32_t s_vocab_times[VOCAB_COUNT] = {0};

// Static pools: at 200+ words these are too large for the 2KB app stack.
static uint16_t s_due_pool[VOCAB_COUNT];
static int s_due_pool_size = 0;

static bool s_notif_enabled = true;
static int s_notif_freq = 60;
static bool s_vibration_enabled = true;
static bool s_diff_basic = true;
static bool s_diff_intermediate = true;
static bool s_diff_advanced = true;

static const int BUCKET_INTERVALS[] = {0, 600, 3600, 14400, 86400, 259200}; // seconds
#define MAX_BUCKET 5

// --- Chunked persistence -----------------------------------------------------
static void write_chunked(uint32_t base_key, const void *data, size_t total) {
  const uint8_t *bytes = data;
  uint32_t key = base_key;
  size_t offset = 0;
  while (offset < total) {
    size_t len = total - offset;
    if (len > CHUNK_BYTES) len = CHUNK_BYTES;
    persist_write_data(key, bytes + offset, len);
    offset += len;
    key++;
  }
}

static void read_chunked(uint32_t base_key, void *data, size_t total) {
  uint8_t *bytes = data;
  uint32_t key = base_key;
  size_t offset = 0;
  while (offset < total) {
    size_t len = total - offset;
    if (len > CHUNK_BYTES) len = CHUNK_BYTES;
    if (!persist_exists(key)) break;
    persist_read_data(key, bytes + offset, len);
    offset += len;
    key++;
  }
}

static void save_srs_data(void) {
  write_chunked(PERSIST_BUCKETS_BASE, s_vocab_buckets, sizeof(s_vocab_buckets));
  write_chunked(PERSIST_TIMES_BASE, s_vocab_times, sizeof(s_vocab_times));
  persist_write_int(PERSIST_SCHEMA_VERSION, SCHEMA_VERSION);
}

static void migrate_v1_data(void) {
  // v1 saved uint8_t[20] at key 100 and time_t[20] (4 bytes each) at key 101.
  // The first 20 entries of the v2 word list are the same words in the same
  // order, so progress carries over directly.
  if (persist_exists(PERSIST_V1_BUCKETS)) {
    uint8_t old_buckets[V1_VOCAB_COUNT] = {0};
    persist_read_data(PERSIST_V1_BUCKETS, old_buckets, sizeof(old_buckets));
    for (int i = 0; i < V1_VOCAB_COUNT && i < (int)VOCAB_COUNT; i++) {
      s_vocab_buckets[i] = old_buckets[i];
    }
    persist_delete(PERSIST_V1_BUCKETS);
  }
  if (persist_exists(PERSIST_V1_TIMES)) {
    int32_t old_times[V1_VOCAB_COUNT] = {0};
    persist_read_data(PERSIST_V1_TIMES, old_times, sizeof(old_times));
    for (int i = 0; i < V1_VOCAB_COUNT && i < (int)VOCAB_COUNT; i++) {
      s_vocab_times[i] = old_times[i];
    }
    persist_delete(PERSIST_V1_TIMES);
  }
  save_srs_data();
}

void state_load_config(void) {
  if (persist_exists(PERSIST_NOTIF_ENABLED)) {
    s_notif_enabled = persist_read_bool(PERSIST_NOTIF_ENABLED);
  }
  if (persist_exists(PERSIST_NOTIF_FREQ)) {
    s_notif_freq = persist_read_int(PERSIST_NOTIF_FREQ);
  }
  if (persist_exists(PERSIST_VIBRATION)) {
    s_vibration_enabled = persist_read_bool(PERSIST_VIBRATION);
  }
  if (persist_exists(PERSIST_DIFF_BASIC)) s_diff_basic = persist_read_bool(PERSIST_DIFF_BASIC);
  if (persist_exists(PERSIST_DIFF_INTERMEDIATE)) s_diff_intermediate = persist_read_bool(PERSIST_DIFF_INTERMEDIATE);
  if (persist_exists(PERSIST_DIFF_ADVANCED)) s_diff_advanced = persist_read_bool(PERSIST_DIFF_ADVANCED);

  if (persist_exists(PERSIST_WORDS_LEARNED)) s_words_learned = persist_read_int(PERSIST_WORDS_LEARNED);
  if (persist_exists(PERSIST_WORDS_REVIEWED)) s_words_reviewed = persist_read_int(PERSIST_WORDS_REVIEWED);
  if (persist_exists(PERSIST_LAUNCH_COUNT)) s_launch_count = persist_read_int(PERSIST_LAUNCH_COUNT);

  int version = persist_exists(PERSIST_SCHEMA_VERSION) ? persist_read_int(PERSIST_SCHEMA_VERSION) : 0;
  if (version >= SCHEMA_VERSION) {
    read_chunked(PERSIST_BUCKETS_BASE, s_vocab_buckets, sizeof(s_vocab_buckets));
    read_chunked(PERSIST_TIMES_BASE, s_vocab_times, sizeof(s_vocab_times));
  } else {
    migrate_v1_data();
  }
}

void state_register_launch(void) {
  s_launch_count++;
  persist_write_int(PERSIST_LAUNCH_COUNT, s_launch_count);
}

int state_get_launch_count(void) { return s_launch_count; }

void state_set_notification_config(bool enabled, int frequency_mins) {
  s_notif_enabled = enabled;
  s_notif_freq = frequency_mins;
  persist_write_bool(PERSIST_NOTIF_ENABLED, enabled);
  persist_write_int(PERSIST_NOTIF_FREQ, frequency_mins);
}

void state_set_vibration_enabled(bool enabled) {
  s_vibration_enabled = enabled;
  persist_write_bool(PERSIST_VIBRATION, enabled);
}

void state_set_difficulty_config(bool basic, bool interm, bool adv) {
  s_diff_basic = basic;
  s_diff_intermediate = interm;
  s_diff_advanced = adv;
  persist_write_bool(PERSIST_DIFF_BASIC, basic);
  persist_write_bool(PERSIST_DIFF_INTERMEDIATE, interm);
  persist_write_bool(PERSIST_DIFF_ADVANCED, adv);
}

bool state_get_diff_basic(void) { return s_diff_basic; }
bool state_get_diff_intermediate(void) { return s_diff_intermediate; }
bool state_get_diff_advanced(void) { return s_diff_advanced; }

bool state_get_notification_enabled(void) { return s_notif_enabled; }
int state_get_notification_frequency(void) { return s_notif_freq; }
bool state_get_vibration_enabled(void) { return s_vibration_enabled; }

// --- Review selection --------------------------------------------------------
static bool difficulty_allowed(int diff) {
  return (diff == 1 && s_diff_basic) ||
         (diff == 2 && s_diff_intermediate) ||
         (diff == 3 && s_diff_advanced);
}

// Rebuild the pool of words that are due now (respecting the difficulty
// filter) and compute the next upcoming due time for the done screen.
static void rebuild_due_pool(void) {
  time_t now = time(NULL);
  s_due_pool_size = 0;
  s_next_due_time = 0;

  for (int i = 0; i < (int)VOCAB_COUNT; i++) {
    if (!difficulty_allowed(vocab_list[i].difficulty)) continue;

    if ((time_t)s_vocab_times[i] <= now) {
      s_due_pool[s_due_pool_size++] = (uint16_t)i;
    } else if (s_next_due_time == 0 || (time_t)s_vocab_times[i] < s_next_due_time) {
      s_next_due_time = (time_t)s_vocab_times[i];
    }
  }
}

static void reset_card_state(void) {
  s_meaning_revealed = false;
  s_display_mode = 0;
  s_minutes_remaining = 10;
}

// Pick a random due word. If avoid_current is true, try not to repeat the
// word currently on screen. Sets the all-caught-up flag when nothing is due.
static void pick_due_word(bool avoid_current) {
  rebuild_due_pool();

  if (s_due_pool_size == 0) {
    s_all_caught_up = true;
    reset_card_state();
    return;
  }

  s_all_caught_up = false;
  int pick = s_due_pool[rand() % s_due_pool_size];
  if (avoid_current && s_due_pool_size > 1) {
    while (pick == s_current_vocab_index) {
      pick = s_due_pool[rand() % s_due_pool_size];
    }
  }
  s_current_vocab_index = pick;
  reset_card_state();
}

void state_init(void) {
  reset_card_state();
  state_jump_to_review();
}

void state_jump_to_review(void) {
  pick_due_word(false);
}

void state_mark_learned(void) {
  if (s_all_caught_up) return;
  if (s_vocab_buckets[s_current_vocab_index] < MAX_BUCKET) {
    s_vocab_buckets[s_current_vocab_index]++;
  }
  int interval = BUCKET_INTERVALS[s_vocab_buckets[s_current_vocab_index]];
  s_vocab_times[s_current_vocab_index] = (int32_t)(time(NULL) + interval);
  s_words_learned++;
  persist_write_int(PERSIST_WORDS_LEARNED, s_words_learned);
  save_srs_data();

  if (s_vibration_enabled) vibes_double_pulse();
  pick_due_word(true);
}

void state_mark_failed(void) {
  if (s_all_caught_up) return;
  s_vocab_buckets[s_current_vocab_index] = 0;
  s_vocab_times[s_current_vocab_index] = (int32_t)time(NULL);
  save_srs_data();

  if (s_vibration_enabled) vibes_short_pulse();
  pick_due_word(true);
}

// Skip = "show me another due word". Respects both the SRS schedule and the
// difficulty filter (v1 walked the raw list sequentially, bypassing both).
void state_next_word(bool auto_advance) {
  pick_due_word(true);
  if (auto_advance && s_vibration_enabled) vibes_double_pulse();
  else if (s_vibration_enabled) vibes_short_pulse();
}

void state_prev_word(void) {
  pick_due_word(true);
  if (s_vibration_enabled) vibes_short_pulse();
}

void state_cycle_mode(void) {
  if (s_all_caught_up) return;
  if (!s_meaning_revealed) {
    s_meaning_revealed = true;
    s_display_mode = 0;
    // A word counts as "reviewed" when its meaning is revealed,
    // not when it is merely skipped past.
    s_words_reviewed++;
    persist_write_int(PERSIST_WORDS_REVIEWED, s_words_reviewed);
  } else {
    s_display_mode = (s_display_mode + 1) % 4;
  }
  if (s_vibration_enabled) vibes_short_pulse();
}

void state_tick(void) {
  // Called once per minute (MINUTE_UNIT keeps battery use low).
  s_minutes_remaining--;
  if (s_minutes_remaining <= 0) {
    state_jump_to_review();
    ui_update_display();
  } else if (s_all_caught_up) {
    // Refresh the done screen countdown; new words may have become due.
    state_jump_to_review();
    ui_update_display();
  }
}

bool state_is_all_caught_up(void) { return s_all_caught_up; }

int state_get_seconds_until_next_due(void) {
  if (s_next_due_time == 0) return 0;
  time_t now = time(NULL);
  if (s_next_due_time <= now) return 0;
  return (int)(s_next_due_time - now);
}

int state_get_bucket(int index) {
  if (index < 0 || index >= (int)VOCAB_COUNT) return 0;
  return s_vocab_buckets[index];
}

int state_get_max_bucket(void) { return MAX_BUCKET; }

int state_get_current_index(void) { return s_current_vocab_index; }
bool state_is_meaning_revealed(void) { return s_meaning_revealed; }
int state_get_display_mode(void) { return s_display_mode; }
int state_get_words_learned(void) { return s_words_learned; }
int state_get_words_reviewed(void) { return s_words_reviewed; }
