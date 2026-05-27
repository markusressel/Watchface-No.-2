#pragma once
#include <pebble.h>

// update layer information
void update_time();

// create the layer
void create_time_layer(
    const Window *window,
    Layer *window_layer,
    GRect window_bounds
);

// destroy the layer
void destroy_time_layer();
