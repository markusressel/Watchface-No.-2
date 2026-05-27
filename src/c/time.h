#pragma once
#include <pebble.h>

// update layer information
void update_time();

// create the layer
void create_time_layer(Layer *window_layer);

// destroy the layer
void destroy_time_layer();
