#pragma once
#include "layer_factory.h"

// update all stepcount layer instances (backward compatible wrapper, called by listeners)
void update_stepcount();

// update a specific stepcount layer instance
void update_stepcount_layer(Layer *layer);

// create the layer (returns the created layer)
Layer *create_stepcount_layer(LayerBuilder builder);

// destroy the layer
void destroy_stepcount_layer(Layer *layer);
