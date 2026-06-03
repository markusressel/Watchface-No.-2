#include "temperature_forecast.h"

#if defined(PBL_PLATFORM_APLITE)

void update_temperature_forecast() {
}

Layer *create_temperature_forecast_layer(LayerBuilder builder) {
    (void) builder;
    return NULL;
}

void destroy_temperature_forecast_layer(Layer *layer) {
    (void) layer;
}

#else

#include <pebble.h>
#include <string.h>

#include "forecast_series.h"
#include "../theme.h"
#include "weather.h"

#define MAX_TEMPERATURE_FORECAST_LAYERS 7
#define MAX_FORECAST_POINTS 24

static Layer *s_layers[MAX_TEMPERATURE_FORECAST_LAYERS];
static int s_layer_count = 0;


static void update_proc(Layer *layer, GContext *ctx) {
    const GRect bounds = layer_get_bounds(layer);
    WeatherData *weather_data = weather_get_data();

    draw_rain_forecast_graph(ctx, bounds, weather_data, MAX_FORECAST_POINTS, GColorBlue);
    draw_temperature_forecast_graph(ctx, bounds, weather_data, MAX_FORECAST_POINTS, theme_get_theme()->WeatherTextColor);
}

void update_temperature_forecast() {
    for (int i = 0; i < s_layer_count; i++) {
        if (s_layers[i] != NULL) {
            layer_mark_dirty(s_layers[i]);
        }
    }
}

Layer *create_temperature_forecast_layer(LayerBuilder builder) {
    if (s_layer_count >= MAX_TEMPERATURE_FORECAST_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max temperature forecast layers exceeded!");
        return NULL;
    }

    Layer *layer = layer_factory_create_custom_layer(builder, update_proc);
    if (layer == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create temperature forecast layer");
        return NULL;
    }

    s_layers[s_layer_count++] = layer;
    layer_mark_dirty(layer);

    return layer;
}

void destroy_temperature_forecast_layer(Layer *layer) {
    if (layer == NULL) {
        return;
    }

    for (int i = 0; i < s_layer_count; i++) {
        if (s_layers[i] == layer) {
            for (int j = i; j < s_layer_count - 1; j++) {
                s_layers[j] = s_layers[j + 1];
            }
            s_layer_count--;
            break;
        }
    }

    layer_destroy(layer);
}

#endif
