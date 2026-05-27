#pragma once
#include "layer_factory.h"

// update all battery bar layer instances (backward compatible wrapper)
void update_battery_bar();

// update a specific battery bar layer instance
void update_battery_bar_layer(Layer *layer);

// create the layer (returns the created layer)
Layer *create_battery_bar_layer(LayerBuilder builder);

// destroy the layer
void destroy_battery_bar_layer(Layer *layer);
