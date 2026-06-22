#include <pebble.h>
#include "ui.h"
#include "state.h"
#include "vocab.h"

static Window *s_main_window;
static TextLayer *s_term_layer;
static TextLayer *s_extra_layer;
static TextLayer *s_hint_layer;

Window* ui_get_main_window(void) {
  return s_main_window;
}

void ui_update_display(void) {
  int idx = state_get_current_index();
  
  text_layer_set_text(s_term_layer, vocab_list[idx].term);
  
  if (!state_is_meaning_revealed()) {
    text_layer_set_text(s_extra_layer, "Press SELECT to reveal...");
    text_layer_set_text(s_hint_layer, "[UP/DN: Skip]");
  } else {
    int mode = state_get_display_mode();
    switch(mode) {
      case 0: text_layer_set_text(s_extra_layer, vocab_list[idx].meaning); break;
      case 1: text_layer_set_text(s_extra_layer, vocab_list[idx].example); break;
      case 2: text_layer_set_text(s_extra_layer, vocab_list[idx].etymology); break;
      case 3: text_layer_set_text(s_extra_layer, vocab_list[idx].phonetic); break;
    }
    text_layer_set_text(s_hint_layer, "[HOLD SELECT: MASTERED]");
  }
}

void ui_update_clock(struct tm *tick_time) {
  // Removed to simplify UI
}

void ui_update_timer(int time_remaining) {
  // Removed to simplify UI
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int w = bounds.size.w;
  int h = bounds.size.h;

  window_set_background_color(window, GColorBlack); // Minimalist deep black

  s_term_layer = text_layer_create(GRect(0, h * 0.1, w, 40));
  text_layer_set_text(s_term_layer, vocab_list[0].term);
  text_layer_set_font(s_term_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_term_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_term_layer, GColorWhite);
  text_layer_set_background_color(s_term_layer, GColorClear);
  text_layer_set_overflow_mode(s_term_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_term_layer));

  int meaning_y = h * 0.35 + 5;
  int meaning_h = h * 0.55;
  s_extra_layer = text_layer_create(GRect(15, meaning_y, w - 30, meaning_h));
  text_layer_set_text(s_extra_layer, "");
  text_layer_set_font(s_extra_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_extra_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_extra_layer, GColorLightGray); // Softer text for reading
  text_layer_set_background_color(s_extra_layer, GColorClear);
  text_layer_set_overflow_mode(s_extra_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_extra_layer));

  int bot_y = meaning_y + meaning_h;
  s_hint_layer = text_layer_create(GRect(0, bot_y, w, h - bot_y));
  text_layer_set_text(s_hint_layer, "[HOLD SELECT: MASTERED]");
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_hint_layer, GColorDarkGray); // Extremely subtle hint
  text_layer_set_background_color(s_hint_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_hint_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_term_layer);
  text_layer_destroy(s_extra_layer);
  text_layer_destroy(s_hint_layer);
}

void ui_init(void) {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  ui_update_display();
}

void ui_deinit(void) {
  window_destroy(s_main_window);
}
