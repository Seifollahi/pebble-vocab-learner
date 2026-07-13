#include <pebble.h>
#include "ui.h"
#include "state.h"
#include "vocab.h"

#define HINT_LAUNCHES 10   // show button hints only for the first N launches
#define FLASH_MS 180

static Window *s_main_window;
static TextLayer *s_term_layer;
static TextLayer *s_extra_layer;
static TextLayer *s_hint_layer;
static Layer *s_bucket_layer;  // Leitner level: five small dots next to the term
static Layer *s_mode_layer;    // display mode: four dots at the bottom edge
static Layer *s_flash_layer;   // grading feedback: brief white flash
static AppTimer *s_flash_timer;

static char s_done_buf[64];

Window* ui_get_main_window(void) {
  return s_main_window;
}

// --- Small drawing helpers ---------------------------------------------------

static void bucket_layer_update(Layer *layer, GContext *ctx) {
  if (state_is_all_caught_up()) return;
  GRect b = layer_get_bounds(layer);
  int max = state_get_max_bucket();
  int level = state_get_bucket(state_get_current_index());
  int spacing = 10;
  int total_w = (max - 1) * spacing;
  int x0 = (b.size.w - total_w) / 2;
  int cy = b.size.h / 2;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  for (int i = 0; i < max; i++) {
    GPoint p = GPoint(x0 + i * spacing, cy);
    if (i < level) {
      graphics_fill_circle(ctx, p, 2);
    } else {
      graphics_draw_circle(ctx, p, 2);
    }
  }
}

static void mode_layer_update(Layer *layer, GContext *ctx) {
  if (state_is_all_caught_up() || !state_is_meaning_revealed()) return;
  GRect b = layer_get_bounds(layer);
  int mode = state_get_display_mode();
  int spacing = 10;
  int total_w = 3 * spacing;
  int x0 = (b.size.w - total_w) / 2;
  int cy = b.size.h / 2;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  for (int i = 0; i < 4; i++) {
    GPoint p = GPoint(x0 + i * spacing, cy);
    if (i == mode) {
      graphics_fill_circle(ctx, p, 2);
    } else {
      graphics_draw_circle(ctx, p, 2);
    }
  }
}

static void flash_layer_update(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void flash_timer_callback(void *data) {
  s_flash_timer = NULL;
  layer_set_hidden(s_flash_layer, true);
}

void ui_flash(void) {
  if (!s_flash_layer) return;
  layer_set_hidden(s_flash_layer, false);
  if (s_flash_timer) app_timer_reschedule(s_flash_timer, FLASH_MS);
  else s_flash_timer = app_timer_register(FLASH_MS, flash_timer_callback, NULL);
}

// --- Content -----------------------------------------------------------------

// Fit longer definitions: drop from Gothic 24 to Gothic 18 when the text
// would overflow its box (matters on 144x168 aplite and round chalk).
static void set_extra_text_fitted(const char *text) {
  GRect frame = layer_get_frame(text_layer_get_layer(s_extra_layer));
  text_layer_set_font(s_extra_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_extra_layer, text);
  GSize content = text_layer_get_content_size(s_extra_layer);
  if (content.h > frame.size.h) {
    text_layer_set_font(s_extra_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  }
}

static void format_next_due(char *buf, size_t len) {
  int secs = state_get_seconds_until_next_due();
  if (secs <= 0) {
    snprintf(buf, len, "All caught up.\nEnable more levels in Settings.");
  } else if (secs < 3600) {
    snprintf(buf, len, "All caught up.\nNext word in %d min.", (secs + 59) / 60);
  } else if (secs < 86400) {
    snprintf(buf, len, "All caught up.\nNext word in %d hr.", (secs + 3599) / 3600);
  } else {
    snprintf(buf, len, "All caught up.\nNext word in %d days.", (secs + 86399) / 86400);
  }
}

void ui_update_display(void) {
  bool show_hints = state_get_launch_count() <= HINT_LAUNCHES;

  if (state_is_all_caught_up()) {
    text_layer_set_text(s_term_layer, "Done!");
    format_next_due(s_done_buf, sizeof(s_done_buf));
    set_extra_text_fitted(s_done_buf);
    text_layer_set_text(s_hint_layer, "");
  } else {
    // The entry's strings live in vocab.c's static record buffer; they stay
    // valid until the next vocab_get(), i.e. until the next display update.
    const VocabEntry *e = vocab_get(state_get_current_index());
    if (!e) {
      text_layer_set_text(s_term_layer, "Error");
      set_extra_text_fitted("Could not read the vocabulary database.");
      text_layer_set_text(s_hint_layer, "");
      layer_mark_dirty(s_bucket_layer);
      layer_mark_dirty(s_mode_layer);
      return;
    }
    text_layer_set_text(s_term_layer, e->term);

    if (!state_is_meaning_revealed()) {
      set_extra_text_fitted("Press SELECT to reveal...");
      text_layer_set_text(s_hint_layer, show_hints ? "SELECT: reveal · UP/DN: skip" : "");
    } else {
      switch (state_get_display_mode()) {
        case 0: set_extra_text_fitted(e->meaning); break;
        case 1: set_extra_text_fitted(e->example); break;
        case 2: set_extra_text_fitted(e->etymology); break;
        case 3: set_extra_text_fitted(e->phonetic); break;
      }
      text_layer_set_text(s_hint_layer, show_hints ? "HOLD SEL: know · DN: forgot" : "");
    }
  }

  layer_mark_dirty(s_bucket_layer);
  layer_mark_dirty(s_mode_layer);
}

// --- Window lifecycle ----------------------------------------------------------

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int w = bounds.size.w;
  int h = bounds.size.h;

  window_set_background_color(window, GColorBlack); // minimalist deep black

  int term_y = PBL_IF_ROUND_ELSE(h * 0.12, h * 0.08);
  s_term_layer = text_layer_create(GRect(0, term_y, w, 40));
  text_layer_set_font(s_term_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_term_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_term_layer, GColorWhite);
  text_layer_set_background_color(s_term_layer, GColorClear);
  text_layer_set_overflow_mode(s_term_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_term_layer));

  s_bucket_layer = layer_create(GRect(0, term_y + 38, w, 8));
  layer_set_update_proc(s_bucket_layer, bucket_layer_update);
  layer_add_child(window_layer, s_bucket_layer);

  int margin = PBL_IF_ROUND_ELSE(24, 15);
  int extra_y = term_y + 50;
  int extra_h = h - extra_y - 28;
  s_extra_layer = text_layer_create(GRect(margin, extra_y, w - 2 * margin, extra_h));
  text_layer_set_font(s_extra_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_extra_layer,
      PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  text_layer_set_text_color(s_extra_layer, GColorLightGray); // softer for reading
  text_layer_set_background_color(s_extra_layer, GColorClear);
  text_layer_set_overflow_mode(s_extra_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_extra_layer));

  s_hint_layer = text_layer_create(GRect(0, h - 26, w, 18));
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_hint_layer, GColorDarkGray); // extremely subtle
  text_layer_set_background_color(s_hint_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_hint_layer));

  s_mode_layer = layer_create(GRect(0, h - 8, w, 8));
  layer_set_update_proc(s_mode_layer, mode_layer_update);
  layer_add_child(window_layer, s_mode_layer);

  s_flash_layer = layer_create(bounds);
  layer_set_update_proc(s_flash_layer, flash_layer_update);
  layer_set_hidden(s_flash_layer, true);
  layer_add_child(window_layer, s_flash_layer);

  ui_update_display();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_term_layer);
  text_layer_destroy(s_extra_layer);
  text_layer_destroy(s_hint_layer);
  layer_destroy(s_bucket_layer);
  layer_destroy(s_mode_layer);
  layer_destroy(s_flash_layer);
}

void ui_init(void) {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
}

void ui_deinit(void) {
  window_destroy(s_main_window);
}
