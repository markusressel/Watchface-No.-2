#pragma once
#include <pebble.h>

// update layer information
void update_battery_bar();

// create the layer
void create_battery_bar_layer(Window *window);

// destroy the layer
void destroy_battery_bar_layer();