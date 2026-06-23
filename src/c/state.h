#pragma once
#include <pebble.h>

void state_init(void);
void state_next_word(bool auto_advance);
void state_prev_word(void);
void state_cycle_mode(void);
void state_mark_learned(void);
void state_mark_failed(void);
void state_jump_to_review(void);

void state_tick(void);

int state_get_time_remaining(void);
int state_get_current_index(void);
bool state_is_meaning_revealed(void);
int state_get_display_mode(void);
int state_get_words_learned(void);
int state_get_words_reviewed(void);

void state_set_notification_config(bool enabled, int frequency_mins);
bool state_get_notification_enabled(void);
int state_get_notification_frequency(void);
void state_set_vibration_enabled(bool enabled);
bool state_get_vibration_enabled(void);
void state_set_difficulty_config(bool basic, bool intermediate, bool advanced);
bool state_get_diff_basic(void);
bool state_get_diff_intermediate(void);
bool state_get_diff_advanced(void);
void state_load_config(void);
