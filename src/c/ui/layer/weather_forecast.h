#pragma once

#include "../../ui/layer_factory.h"

void update_weather_forecast();

void weather_forecast_layer_update_settings();

void weather_forecast_tick_update();

Layer *create_weather_forecast_layer(LayerBuilder builder);

void destroy_weather_forecast_layer(Layer *layer);
