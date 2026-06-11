#pragma once

#include "../../ui/layer_factory.h"

void update_temperature_forecast();

Layer *create_temperature_forecast_layer(LayerBuilder builder);

void destroy_temperature_forecast_layer(Layer * layer);