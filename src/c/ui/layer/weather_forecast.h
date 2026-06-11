#pragma once

#include "../../ui/layer_factory.h"

void update_weather_forecast();

Layer *create_weather_forecast_layer(LayerBuilder builder);

void destroy_weather_forecast_layer(Layer *layer);
