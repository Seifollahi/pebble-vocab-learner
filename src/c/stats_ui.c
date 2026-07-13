#include <pebble.h>
#include "stats_ui.h"
#include "state.h"
#include "vocab.h"

static Window *s_stats_window;
static TextLayer *s_title_layer;
static TextLayer *s_learned_layer;
static TextLayer *s_reviewed_layer;
static TextLayer *s_graduated_layer;

static void stats_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int margin = PBL_IF_ROUND_ELSE(30, 10);
  GTextAlignment align = PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft);

  window_set_background_color(window, GColorBlack);

  s_title_layer = text_layer_create(GRect(0, 10, bounds.size.w, 40));
  text_layer_set_text(s_title_layer, "Learning Stats");
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_background_color(s_title_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_learned_layer = text_layer_create(GRect(margin, 55, bounds.size.w - 2 * margin, 32));
  text_layer_set_font(s_learned_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_learned_layer, align);
  text_layer_set_text_color(s_learned_layer, GColorLightGray);
  text_layer_set_background_color(s_learned_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_learned_layer));

  s_reviewed_layer = text_layer_create(GRect(margin, 87, bounds.size.w - 2 * margin, 32));
  text_layer_set_font(s_reviewed_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_reviewed_layer, align);
  text_layer_set_text_color(s_reviewed_layer, GColorLightGray);
  text_layer_set_background_color(s_reviewed_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_reviewed_layer));

  s_graduated_layer = text_layer_create(GRect(margin, 119, bounds.size.w - 2 * margin, 32));
  text_layer_set_font(s_graduated_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_graduated_layer, align);
  text_layer_set_text_color(s_graduated_layer, GColorLightGray);
  text_layer_set_background_color(s_graduated_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_graduated_layer));
}

static void stats_window_appear(Window *window) {
  static char s_learned_buf[32];
  static char s_reviewed_buf[32];
  static char s_graduated_buf[32];

  // Stats are persisted across launches as of v2.
  snprintf(s_learned_buf, sizeof(s_learned_buf), "Mastered: %d", state_get_words_learned());
  text_layer_set_text(s_learned_layer, s_learned_buf);

  snprintf(s_reviewed_buf, sizeof(s_reviewed_buf), "Reviewed: %d", state_get_words_reviewed());
  text_layer_set_text(s_reviewed_layer, s_reviewed_buf);

  int graduated = 0;
  for (int i = 0; i < vocab_count(); i++) {
    if (state_get_bucket(i) >= state_get_max_bucket()) graduated++;
  }
  snprintf(s_graduated_buf, sizeof(s_graduated_buf), "Graduated: %d/%d", graduated, vocab_count());
  text_layer_set_text(s_graduated_layer, s_graduated_buf);
}

static void stats_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_learned_layer);
  text_layer_destroy(s_reviewed_layer);
  text_layer_destroy(s_graduated_layer);
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
