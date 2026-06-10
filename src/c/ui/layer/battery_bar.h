#pragma once
#include "../ui/layer_factory.h"

// update all battery bar layer instances (backward compatible wrapper)
void update_battery_bar();

// update a specific battery bar layer instance
void update_battery_bar_layer(Layer * layer);

// create the layer (returns the created layer)
Layer *create_battery_bar_layer(LayerBuilder builder);

// set battery bar horizontal alignment for a specific layer
void battery_bar_layer_set_horizontal_alignment(Layer *layer, HorizontalAlignment alignment);

// set battery bar vertical alignment for a specific layer
void battery_bar_layer_set_vertical_alignment(Layer *layer, VerticalAlignment alignment);

// destroy the layer
void destroy_battery_bar_layer(Layer * layer);