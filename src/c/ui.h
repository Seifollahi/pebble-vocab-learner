#pragma once
#include <pebble.h>

void ui_init(void);
void ui_deinit(void);
void ui_update_display(void);
void ui_update_clock(struct tm *tick_time);
void ui_update_timer(int time_remaining);
Window* ui_get_main_window(void);
