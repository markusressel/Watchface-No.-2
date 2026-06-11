#pragma once
#include "../../ui/layer_factory.h"

// update all stepcount layer instances (backward compatible wrapper, called by listeners)
void update_stepcount();

// create the layer (returns the created layer)
Layer *create_stepcount_layer(LayerBuilder builder);

// destroy the layer
void destroy_stepcount_layer(Layer * layer);