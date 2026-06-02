#pragma once
#include "../layer_factory.h"

// update all heartrate layer instances (called by listeners)
void update_heartrate();

// create the layer (returns the created layer)
Layer *create_heartrate_layer(LayerBuilder builder);

// destroy the layer
void destroy_heartrate_layer(Layer *layer);

