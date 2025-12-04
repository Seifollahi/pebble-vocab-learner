#include <pebble.h>
#include "vocab.h"

static Window *s_main_window;
static TextLayer *s_title_layer;
static TextLayer *s_term_layer;
static TextLayer *s_extra_layer;
static TextLayer *s_counter_layer;
static TextLayer *s_hint_layer;
static TextLayer *s_clock_layer;
static TextLayer *s_stats_layer;
static TextLayer *s_category_layer;
static int s_current_vocab_index = 0;
static int s_time_remaining = 600;
static bool s_meaning_revealed = false;
static int s_display_mode = 0;  // 0=meaning, 1=example, 2=etymology, 3=phonetic
static int s_words_learned = 0;
static int s_words_reviewed = 0;
static int s_streak_days = 1;

// Called every second to update countdown
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_time_remaining--;
  
  // Update clock display
  static char clock_buffer[16];
  strftime(clock_buffer, sizeof(clock_buffer), "%H:%M:%S", tick_time);
  text_layer_set_text(s_clock_layer, clock_buffer);
  
  // Update counter display
  static char counter_buffer[32];
  int minutes = s_time_remaining / 60;
  int seconds = s_time_remaining % 60;
  snprintf(counter_buffer, sizeof(counter_buffer), "Next in %d:%02d", minutes, seconds);
  text_layer_set_text(s_counter_layer, counter_buffer);
  
  // When 10 minutes have passed, show next vocabulary
  if (s_time_remaining <= 0) {
    s_current_vocab_index = (s_current_vocab_index + 1) % VOCAB_COUNT;
    s_time_remaining = 600;
    s_meaning_revealed = false;
    s_display_mode = 0;
    s_words_learned++;
    
    // Update term and hide info
    text_layer_set_text(s_term_layer, vocab_list[s_current_vocab_index].term);
    text_layer_set_text(s_extra_layer, "");
    text_layer_set_text(s_hint_layer, "[MIDDLE: reveal | UP/DOWN: nav]");
    
    // Update category/difficulty
    static char diff_buf[48];
    const char *diff_text = vocab_list[s_current_vocab_index].difficulty == 1 ? "Beginner" : 
                            vocab_list[s_current_vocab_index].difficulty == 2 ? "Intermediate" : "Advanced";
    snprintf(diff_buf, sizeof(diff_buf), "%s | %s", vocab_list[s_current_vocab_index].category, diff_text);
    text_layer_set_text(s_category_layer, diff_buf);
    
    // Update stats
    static char stats_buf[48];
    snprintf(stats_buf, sizeof(stats_buf), "Learned: %d | Reviewed: %d", s_words_learned, s_words_reviewed);
    text_layer_set_text(s_stats_layer, stats_buf);
    
    // Vibrate with different pattern
    vibes_double_pulse();
  }
}

// Handle UP button - next word
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_current_vocab_index = (s_current_vocab_index + 1) % VOCAB_COUNT;
  s_time_remaining = 600;
  s_meaning_revealed = false;
  s_display_mode = 0;
  s_words_reviewed++;
  
  // Update display
  text_layer_set_text(s_term_layer, vocab_list[s_current_vocab_index].term);
  text_layer_set_text(s_extra_layer, "");
  text_layer_set_text(s_hint_layer, "[MIDDLE: reveal | UP/DOWN: nav]");
  
  // Update category/difficulty
  static char diff_buf[48];
  const char *diff_text = vocab_list[s_current_vocab_index].difficulty == 1 ? "Beginner" : 
                          vocab_list[s_current_vocab_index].difficulty == 2 ? "Intermediate" : "Advanced";
  snprintf(diff_buf, sizeof(diff_buf), "%s | %s", vocab_list[s_current_vocab_index].category, diff_text);
  text_layer_set_text(s_category_layer, diff_buf);
  
  // Update stats
  static char stats_buf[48];
  snprintf(stats_buf, sizeof(stats_buf), "Learned: %d | Reviewed: %d", s_words_learned, s_words_reviewed);
  text_layer_set_text(s_stats_layer, stats_buf);
  
  vibes_short_pulse();
}

// Handle DOWN button - previous word
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_current_vocab_index = (s_current_vocab_index - 1 + VOCAB_COUNT) % VOCAB_COUNT;
  s_time_remaining = 600;
  s_meaning_revealed = false;
  s_display_mode = 0;
  s_words_reviewed++;
  
  // Update display
  text_layer_set_text(s_term_layer, vocab_list[s_current_vocab_index].term);
  text_layer_set_text(s_extra_layer, "");
  text_layer_set_text(s_hint_layer, "[MIDDLE: reveal | UP/DOWN: nav]");
  
  // Update category/difficulty
  static char diff_buf[48];
  const char *diff_text = vocab_list[s_current_vocab_index].difficulty == 1 ? "Beginner" : 
                          vocab_list[s_current_vocab_index].difficulty == 2 ? "Intermediate" : "Advanced";
  snprintf(diff_buf, sizeof(diff_buf), "%s | %s", vocab_list[s_current_vocab_index].category, diff_text);
  text_layer_set_text(s_category_layer, diff_buf);
  
  // Update stats
  static char stats_buf[48];
  snprintf(stats_buf, sizeof(stats_buf), "Learned: %d | Reviewed: %d", s_words_learned, s_words_reviewed);
  text_layer_set_text(s_stats_layer, stats_buf);
  
  vibes_short_pulse();
}

// Handle SELECT (middle) button - cycle through: meaning -> example -> etymology -> phonetic
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!s_meaning_revealed) {
    s_meaning_revealed = true;
    s_display_mode = 0;
    text_layer_set_text(s_extra_layer, vocab_list[s_current_vocab_index].meaning);
    text_layer_set_text(s_hint_layer, "[MIDDLE: cycle | UP/DOWN: nav]");
    vibes_short_pulse();
  } else {
    s_display_mode = (s_display_mode + 1) % 4;
    
    switch(s_display_mode) {
      case 0:
        text_layer_set_text(s_extra_layer, vocab_list[s_current_vocab_index].meaning);
        break;
      case 1:
        text_layer_set_text(s_extra_layer, vocab_list[s_current_vocab_index].example);
        break;
      case 2:
        text_layer_set_text(s_extra_layer, vocab_list[s_current_vocab_index].etymology);
        break;
      case 3:
        text_layer_set_text(s_extra_layer, vocab_list[s_current_vocab_index].phonetic);
        break;
    }
    vibes_short_pulse();
  }
}

// Configure click handlers
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int y_offset = 5; // Initial y-offset for the first layer

  window_set_background_color(window, GColorOxfordBlue); // Subtle dark blue background for modern look

  // Draw a separator line function
  void add_separator(int y) {
    Layer *sep = layer_create(GRect(10, y, bounds.size.w - 20, 2));
    layer_set_update_proc(sep, [](Layer *layer, GContext *ctx) {
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_fill_rect(ctx, layer_get_bounds(layer), 1, GCornersAll);
    });
    layer_add_child(window_layer, sep);
  }

  // Title layer
  s_title_layer = text_layer_create(GRect(5, y_offset, bounds.size.w - 10, 18));
  text_layer_set_text(s_title_layer, "VOCABULARY");
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_title_layer, GColorChromeYellow);
  text_layer_set_background_color(s_title_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));
  y_offset += 20;
  add_separator(y_offset);
  y_offset += 6;

  // Term highlight box
  Layer *term_box = layer_create(GRect(10, y_offset, bounds.size.w - 20, 40));
  layer_set_update_proc(term_box, [](Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorVividCerulean);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 6, GCornersAll);
  });
  layer_add_child(window_layer, term_box);

  s_term_layer = text_layer_create(GRect(14, y_offset + 4, bounds.size.w - 28, 32));
  text_layer_set_text(s_term_layer, vocab_list[0].term);
  text_layer_set_font(s_term_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_term_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_term_layer, GColorWhite);
  text_layer_set_background_color(s_term_layer, GColorClear);
  text_layer_set_overflow_mode(s_term_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_term_layer));
  y_offset += 46;
  add_separator(y_offset);
  y_offset += 6;

  // Meaning highlight box
  Layer *meaning_box = layer_create(GRect(10, y_offset, bounds.size.w - 20, 32));
  layer_set_update_proc(meaning_box, [](Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorSpringBud);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
  });
  layer_add_child(window_layer, meaning_box);

  s_extra_layer = text_layer_create(GRect(14, y_offset + 4, bounds.size.w - 28, 24));
  text_layer_set_text(s_extra_layer, "");
  text_layer_set_font(s_extra_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_extra_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_extra_layer, GColorBlack);
  text_layer_set_background_color(s_extra_layer, GColorClear);
  text_layer_set_overflow_mode(s_extra_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_extra_layer));
  y_offset += 38;
  add_separator(y_offset);
  y_offset += 6;

  // Category/Difficulty layer
  s_category_layer = text_layer_create(GRect(10, y_offset, bounds.size.w - 20, 16));
  text_layer_set_text(s_category_layer, "General | Beginner");
  text_layer_set_font(s_category_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_category_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_category_layer, GColorLimerick);
  text_layer_set_background_color(s_category_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_category_layer));

  // Stats layer
  s_stats_layer = text_layer_create(GRect(10, y_offset, bounds.size.w - 20, 16));
  text_layer_set_text(s_stats_layer, "Learned: 0 | Reviewed: 0");
  text_layer_set_font(s_stats_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_stats_layer, GTextAlignmentRight);
  text_layer_set_text_color(s_stats_layer, GColorMalachite);
  text_layer_set_background_color(s_stats_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_stats_layer));
  y_offset += 18;

  // Clock layer
  s_clock_layer = text_layer_create(GRect(10, y_offset, bounds.size.w - 20, 16));
  text_layer_set_text(s_clock_layer, "00:00:00");
  text_layer_set_font(s_clock_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_clock_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_clock_layer, GColorCyan);
  text_layer_set_background_color(s_clock_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_clock_layer));

  // Countdown layer
  s_counter_layer = text_layer_create(GRect(10, y_offset, bounds.size.w - 20, 16));
  text_layer_set_text(s_counter_layer, "Next in: 10:00");
  text_layer_set_font(s_counter_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_counter_layer, GTextAlignmentRight);
  text_layer_set_text_color(s_counter_layer, GColorRed);
  text_layer_set_background_color(s_counter_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_counter_layer));
  y_offset += 18;

  // Hint layer (button instructions)
  s_hint_layer = text_layer_create(GRect(10, y_offset, bounds.size.w - 20, 16));
  text_layer_set_text(s_hint_layer, "[MIDDLE: reveal | UP/DOWN: nav]");
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_hint_layer, GColorOrange);
  text_layer_set_background_color(s_hint_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_hint_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_term_layer);
  text_layer_destroy(s_extra_layer);
  text_layer_destroy(s_counter_layer);
  text_layer_destroy(s_hint_layer);
  text_layer_destroy(s_clock_layer);
  text_layer_destroy(s_stats_layer);
  text_layer_destroy(s_category_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
