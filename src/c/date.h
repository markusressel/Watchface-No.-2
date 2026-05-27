#pragma once

// update layer information
void update_date();

// create the layer
void create_date_layer(
    const Window *window,
    Layer *window_layer,
    GRect window_bounds
);

// destroy the layer
void destroy_date_layer();
