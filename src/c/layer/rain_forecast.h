#pragma once

#include "../layer_factory.h"

void update_rain_forecast();
Layer *create_rain_forecast_layer(LayerBuilder builder);
void destroy_rain_forecast_layer(Layer *layer);

