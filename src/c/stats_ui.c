#include <pebble.h>
#include "stats_ui.h"
#include "state.h"

static Window *s_stats_window;
static TextLayer *s_title_layer;
static TextLayer *s_learned_layer;
static TextLayer *s_reviewed_layer;

static void stats_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorBlack);

  s_title_layer = text_layer_create(GRect(0, 10, bounds.size.w, 40));
  text_layer_set_text(s_title_layer, "Learning Stats");
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_background_color(s_title_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_learned_layer = text_layer_create(GRect(10, 60, bounds.size.w - 20, 40));
  text_layer_set_font(s_learned_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_learned_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_learned_layer, GColorLightGray);
  text_layer_set_background_color(s_learned_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_learned_layer));

  s_reviewed_layer = text_layer_create(GRect(10, 100, bounds.size.w - 20, 40));
  text_layer_set_font(s_reviewed_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_reviewed_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_reviewed_layer, GColorLightGray);
  text_layer_set_background_color(s_reviewed_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_reviewed_layer));
}

static void stats_window_appear(Window *window) {
  static char s_learned_buf[32];
  static char s_reviewed_buf[32];
  
  snprintf(s_learned_buf, sizeof(s_learned_buf), "Mastered: %d", state_get_words_learned());
  text_layer_set_text(s_learned_layer, s_learned_buf);
  
  snprintf(s_reviewed_buf, sizeof(s_reviewed_buf), "Reviewed: %d", state_get_words_reviewed());
  text_layer_set_text(s_reviewed_layer, s_reviewed_buf);
}

static void stats_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_learned_layer);
  text_layer_destroy(s_reviewed_layer);
}

void stats_ui_init(void) {
  s_stats_window = window_create();
  window_set_window_handlers(s_stats_window, (WindowHandlers) {
    .load = stats_window_load,
    .appear = stats_window_appear,
    .unload = stats_window_unload
  });
}

void stats_ui_deinit(void) {
  window_destroy(s_stats_window);
}

void stats_ui_push(void) {
  window_stack_push(s_stats_window, true);
}
