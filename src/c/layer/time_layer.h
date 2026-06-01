#pragma once
#include "../layer_factory.h"

// Update all time layer instances (called by listeners).
void update_time_layer();

// Update a specific time layer instance.
void update_time_layer_layer(Layer * layer);

// Create the dotted time layer.
Layer *create_time_layer_layer(LayerBuilder builder);

// Destroy a previously created dotted time layer.
void destroy_time_layer_layer(Layer * layer);