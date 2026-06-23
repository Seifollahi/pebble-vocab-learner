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
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  state_tick();
  ui_update_timer(state_get_time_remaining());
  ui_update_clock(tick_time);
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

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  state_cycle_mode();
  ui_update_display();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  state_mark_learned();
  ui_update_display();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP, 500, up_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void init(void) {
  srand(time(NULL));
  wakeup_cancel_all();
  state_load_config();
  state_init();
  ui_init();
  stats_ui_init();
  
  window_set_click_config_provider(ui_get_main_window(), click_config_provider);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(256, 256);
  
  if (launch_reason() == APP_LAUNCH_TIMELINE_ACTION) {
    uint32_t launch_code = launch_get_args();
    if (launch_code == 1) {
      state_next_word(true);
      ui_update_display();
    }
  }
  
  // Request pins if enabled
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) == APP_MSG_OK) {
    dict_write_uint8(iter, MESSAGE_KEY_NOTIFICATIONS_ENABLED, state_get_notification_enabled() ? 1 : 0);
    dict_write_uint32(iter, MESSAGE_KEY_NOTIFICATIONS_FREQUENCY, state_get_notification_frequency());
    app_message_outbox_send();
  }
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
