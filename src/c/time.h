#pragma once
#include "layer_factory.h"

// update all time layer instances (backward compatible wrapper, called by listeners)
void update_time();

// update a specific time layer instance
void update_time_layer(Layer *layer);

// create the layer (returns the created layer)
Layer *create_time_layer(LayerBuilder builder);

// destroy the layer
void destroy_time_layer(Layer *layer);
