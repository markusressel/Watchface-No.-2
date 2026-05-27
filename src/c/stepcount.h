#pragma once

// update layer information
void update_stepcount();

// create the layer
void create_stepcount_layer(
    const Window *window,
    Layer *window_layer,
    GRect window_bounds,
    int offsetX,
    int offsetY
);

// destroy the layer
void destroy_stepcount_layer();
