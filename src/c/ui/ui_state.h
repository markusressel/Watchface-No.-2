#pragma once

#include <pebble.h>
#include "watch_layout.h"

void ui_state_init(Window *window);

void ui_state_deinit();

void ui_state_create_layers(WatchLayout *layout);

void ui_state_destroy_layers();

int ui_state_get_row_count(void);

Layer *ui_state_get_layer(int index);

WidgetId ui_state_get_widget_id(int index);
