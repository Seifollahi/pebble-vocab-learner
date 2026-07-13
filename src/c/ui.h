#pragma once
#include <pebble.h>

void ui_init(void);
void ui_deinit(void);
void ui_update_display(void);
void ui_flash(void); // brief full-screen flash as grading feedback
Window* ui_get_main_window(void);
