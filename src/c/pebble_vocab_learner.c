#include <pebble.h>
#include "ui.h"
#include "state.h"

#define WAKEUP_REASON 0

static void schedule_wakeup(void) {
  wakeup_cancel_all();
  if (state_get_notification_enabled()) {
    time_t future_time = time(NULL) + (state_get_notification_frequency() * 60);
    wakeup_schedule(future_time, WAKEUP_REASON, true);
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *enabled_tuple = dict_find(iter, MESSAGE_KEY_NOTIFICATIONS_ENABLED);
  Tuple *freq_tuple = dict_find(iter, MESSAGE_KEY_NOTIFICATIONS_FREQUENCY);
  Tuple *vib_tuple = dict_find(iter, MESSAGE_KEY_VIBRATION_ENABLED);
  
  if (enabled_tuple && freq_tuple) {
    bool enabled = enabled_tuple->value->int32 == 1;
    int freq = freq_tuple->value->int32;
    state_set_notification_config(enabled, freq);
    schedule_wakeup();
  }
  
  if (vib_tuple) {
    bool vib_enabled = vib_tuple->value->int32 == 1;
    state_set_vibration_enabled(vib_enabled);
  }
}

static void wakeup_handler(WakeupId id, int32_t reason) {
  state_next_word(true);
  ui_update_display();
  schedule_wakeup();
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
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void init(void) {
  state_load_config();
  state_init();
  ui_init();
  
  window_set_click_config_provider(ui_get_main_window(), click_config_provider);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(64, 64);
  
  wakeup_service_subscribe(wakeup_handler);
  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    WakeupId id = 0;
    int32_t reason = 0;
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  } else {
    schedule_wakeup();
  }
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  ui_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
