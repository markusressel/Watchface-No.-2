#pragma once
#include "layer_factory.h"

// update all date layer instances (backward compatible wrapper, called by listeners)
void update_date();

// update a specific date layer instance
void update_date_layer(Layer *layer);

// create the layer (returns the created layer)
Layer *create_date_layer(LayerBuilder builder);

// destroy the layer
void destroy_date_layer(Layer *layer);
