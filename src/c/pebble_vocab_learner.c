#include <pebble.h>
#include <stdlib.h>
#include "ui.h"
#include "state.h"
#include "stats_ui.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *enabled_tuple = dict_find(iter, MESSAGE_KEY_NOTIFICATIONS_ENABLED);
  Tuple *freq_tuple = dict_find(iter, MESSAGE_KEY_NOTIFICATIONS_FREQUENCY);
  Tuple *vib_tuple = dict_find(iter, MESSAGE_KEY_VIBRATION_ENABLED);

  Tuple *diff_b = dict_find(iter, MESSAGE_KEY_DIFF_BASIC);
  Tuple *diff_i = dict_find(iter, MESSAGE_KEY_DIFF_INTERMEDIATE);
  Tuple *diff_a = dict_find(iter, MESSAGE_KEY_DIFF_ADVANCED);

  if (enabled_tuple && freq_tuple) {
    bool enabled = enabled_tuple->value->int32 == 1;
    int freq = freq_tuple->value->int32;
    state_set_notification_config(enabled, freq);
  }

  if (vib_tuple) {
    bool vib_enabled = vib_tuple->value->int32 == 1;
    state_set_vibration_enabled(vib_enabled);
  }

  if (diff_b && diff_i && diff_a) {
    state_set_difficulty_config(diff_b->value->int32 == 1,
                                diff_i->value->int32 == 1,
                                diff_a->value->int32 == 1);
    // The difficulty filter changed, so the current word may no longer apply.
    state_jump_to_review();
    ui_update_display();
  }
}

// MINUTE_UNIT instead of SECOND_UNIT: waking the CPU once a minute instead of
// once a second is a meaningful battery win, and nothing on screen ticks.
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  state_tick();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  state_next_word(false);
  ui_update_display();
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  stats_ui_push();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  state_prev_word();
  ui_update_display();
}

// HOLD DOWN = "I forgot this word". Resets its Leitner bucket so the SRS can
// actually demote words (v1 could only ever promote them).
static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  state_mark_failed();
  ui_update_display();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  state_cycle_mode();
  ui_update_display();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  state_mark_learned();
  ui_flash(); // visual confirmation that the grade registered
  ui_update_display();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP, 500, up_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 500, down_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void init(void) {
  srand(time(NULL));
  wakeup_cancel_all();
  state_load_config();
  state_register_launch();
  state_init();
  ui_init();
  stats_ui_init();

  window_set_click_config_provider(ui_get_main_window(), click_config_provider);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(256, 256);

  if (launch_reason() == APP_LAUNCH_TIMELINE_ACTION) {
    uint32_t launch_code = launch_get_args();
    if (launch_code == 1) {
      state_jump_to_review();
      ui_update_display();
    }
  }

  // Note: timeline pins are pushed by the JS side only when settings are
  // saved (v1 re-requested pins on every launch, which spammed the timeline).
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  stats_ui_deinit();
  ui_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
