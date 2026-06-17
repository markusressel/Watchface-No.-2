#pragma once

#include <pebble.h>

/**
 * Creates the status layer and adds it as a child to the given parent layer.
 * 
 * @param parent_layer The parent layer (typically the window root layer).
 * @return The created status layer.
 */
Layer *status_layer_create(Layer *parent_layer);

/**
 * Destroys the status layer.
 */
void status_layer_destroy();

/**
 * Marks the status layer as dirty, triggering a redraw.
 */
void status_layer_update();
