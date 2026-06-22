#include "state.h"
#include "vocab.h"
#include "ui.h"

#define PERSIST_NOTIF_ENABLED 1
#define PERSIST_NOTIF_FREQ 2
#define PERSIST_VIBRATION 3
#define PERSIST_SRS_BUCKETS 100
#define PERSIST_SRS_TIMES 101

static int s_current_vocab_index = 0;
static int s_time_remaining = 600;
static bool s_meaning_revealed = false;
static int s_display_mode = 0;
static int s_words_learned = 0;
static int s_words_reviewed = 0;

static uint8_t s_vocab_buckets[VOCAB_COUNT] = {0};
static time_t s_vocab_times[VOCAB_COUNT] = {0};

static bool s_notif_enabled = true;
static int s_notif_freq = 60;
static bool s_vibration_enabled = true;

static const int BUCKET_INTERVALS[] = {0, 600, 3600, 14400, 86400, 259200}; // in seconds
#define MAX_BUCKET 5

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
  
  if (persist_exists(PERSIST_SRS_BUCKETS)) {
    persist_read_data(PERSIST_SRS_BUCKETS, s_vocab_buckets, sizeof(s_vocab_buckets));
  }
  if (persist_exists(PERSIST_SRS_TIMES)) {
    persist_read_data(PERSIST_SRS_TIMES, s_vocab_times, sizeof(s_vocab_times));
  }
}

static void save_srs_data(void) {
  persist_write_data(PERSIST_SRS_BUCKETS, s_vocab_buckets, sizeof(s_vocab_buckets));
  persist_write_data(PERSIST_SRS_TIMES, s_vocab_times, sizeof(s_vocab_times));
}

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

bool state_get_notification_enabled(void) { return s_notif_enabled; }
int state_get_notification_frequency(void) { return s_notif_freq; }
bool state_get_vibration_enabled(void) { return s_vibration_enabled; }

void state_init(void) {
  s_current_vocab_index = 0;
  s_time_remaining = 600;
  s_meaning_revealed = false;
  s_display_mode = 0;
  state_jump_to_review();
}

void state_jump_to_review(void) {
  time_t now = time(NULL);
  int best_idx = s_current_vocab_index;
  bool found = false;
  
  for (int i = 0; i < (int)VOCAB_COUNT; i++) {
    int idx = (s_current_vocab_index + i) % (int)VOCAB_COUNT;
    if (s_vocab_times[idx] <= now) {
      best_idx = idx;
      found = true;
      break;
    }
  }
  
  if (!found) {
    // Just find the earliest one
    time_t earliest = s_vocab_times[s_current_vocab_index];
    for(int i=0; i<(int)VOCAB_COUNT; i++) {
      if(s_vocab_times[i] < earliest) {
        earliest = s_vocab_times[i];
        best_idx = i;
      }
    }
  }
  
  s_current_vocab_index = best_idx;
  s_meaning_revealed = false;
  s_display_mode = 0;
}

void state_mark_learned(void) {
  if (s_vocab_buckets[s_current_vocab_index] < MAX_BUCKET) {
    s_vocab_buckets[s_current_vocab_index]++;
  }
  int interval = BUCKET_INTERVALS[s_vocab_buckets[s_current_vocab_index]];
  s_vocab_times[s_current_vocab_index] = time(NULL) + interval;
  s_words_learned++;
  save_srs_data();
  
  if (s_vibration_enabled) vibes_double_pulse();
  state_jump_to_review();
}

void state_mark_failed(void) {
  s_vocab_buckets[s_current_vocab_index] = 0;
  s_vocab_times[s_current_vocab_index] = time(NULL);
  save_srs_data();
  
  if (s_vibration_enabled) vibes_short_pulse();
  state_jump_to_review();
}

void state_next_word(bool auto_advance) {
  s_current_vocab_index = (s_current_vocab_index + 1) % VOCAB_COUNT;
  s_time_remaining = 600;
  s_meaning_revealed = false;
  s_display_mode = 0;
  s_words_reviewed++;
  if (auto_advance && s_vibration_enabled) vibes_double_pulse();
  else if (s_vibration_enabled) vibes_short_pulse();
}

void state_prev_word(void) {
  s_current_vocab_index = (s_current_vocab_index - 1 + VOCAB_COUNT) % VOCAB_COUNT;
  s_time_remaining = 600;
  s_meaning_revealed = false;
  s_display_mode = 0;
  s_words_reviewed++;
  if (s_vibration_enabled) vibes_short_pulse();
}

void state_cycle_mode(void) {
  if (!s_meaning_revealed) {
    s_meaning_revealed = true;
    s_display_mode = 0;
  } else {
    s_display_mode = (s_display_mode + 1) % 4;
  }
  if (s_vibration_enabled) vibes_short_pulse();
}

void state_tick(void) {
  s_time_remaining--;
  if (s_time_remaining <= 0) {
    state_jump_to_review();
    s_time_remaining = 600;
    ui_update_display();
  }
}

int state_get_time_remaining(void) { return s_time_remaining; }
int state_get_current_index(void) { return s_current_vocab_index; }
bool state_is_meaning_revealed(void) { return s_meaning_revealed; }
int state_get_display_mode(void) { return s_display_mode; }
int state_get_words_learned(void) { return s_words_learned; }
int state_get_words_reviewed(void) { return s_words_reviewed; }
