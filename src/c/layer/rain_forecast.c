#include "rain_forecast.h"

#if defined(PBL_PLATFORM_APLITE)

void update_rain_forecast() {
}

Layer *create_rain_forecast_layer(LayerBuilder builder) {
    (void) builder;
    return NULL;
}

void destroy_rain_forecast_layer(Layer *layer) {
    (void) layer;
}

#else

#include <pebble.h>
#include <stdlib.h>

#include "../clay_settings.h"
#include "forecast_series.h"
#include "graph_utils.h"
#include "../theme.h"
#include "weather.h"

#define MAX_RAIN_FORECAST_LAYERS 7
#define MAX_FORECAST_POINTS 24

static Layer *s_layers[MAX_RAIN_FORECAST_LAYERS];
static int s_layer_count = 0;

static GColor rain_color_for_value(
    const int value,
    const int min_value,
    const int max_value,
    void *context
) {
    (void) context;

#if defined(PBL_COLOR)
    if (max_value <= min_value) {
        return theme_get_theme()->WeatherTextColor;
    }

    const int percent = ((value - min_value) * 100) / (max_value - min_value);
    if (percent >= 70) {
        return GColorBlueMoon;
    }
    if (percent >= 35) {
        return GColorPictonBlue;
    }
    return GColorLightGray;
#else
    (void) value;
    (void) min_value;
    (void) max_value;
    return theme_get_theme()->WeatherTextColor;
#endif
}

static void update_proc(Layer *layer, GContext *ctx) {
    const GRect bounds = layer_get_bounds(layer);
    WeatherData *weather_data = weather_get_data();
    ClaySettings *settings = clay_get_settings();

    const int dot_size = settings->DotHeight > 1 ? settings->DotHeight : 1;

    int values[MAX_FORECAST_POINTS] = {0};
    int value_count = forecast_parse_int_series(
        weather_data->RainForecastMmX10Encoded,
        sizeof(weather_data->RainForecastMmX10Encoded),
        values,
        MAX_FORECAST_POINTS
    );
    if (value_count <= 0) {
        values[0] = weather_data->RainNextHourMmX10;
        value_count = 1;
    }

    GraphDrawConfig graph_config = {
        .graph_type = GRAPH_TYPE_BAR,
        .dot_size = dot_size,
        .interpolation_steps = settings->DotHorizontalGap,
        .bars_from_zero = true,
        .default_color = theme_get_theme()->WeatherTextColor,
        .color_for_value = rain_color_for_value,
        .color_context = NULL,
    };

    graph_draw_series(ctx, bounds, values, value_count, &graph_config);
}

void update_rain_forecast() {
    for (int i = 0; i < s_layer_count; i++) {
        if (s_layers[i] != NULL) {
            layer_mark_dirty(s_layers[i]);
        }
    }
}

Layer *create_rain_forecast_layer(LayerBuilder builder) {
    if (s_layer_count >= MAX_RAIN_FORECAST_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max rain forecast layers exceeded!");
        return NULL;
    }

    Layer *layer = layer_factory_create_custom_layer(builder, update_proc);
    if (layer == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create rain forecast layer");
        return NULL;
    }

    s_layers[s_layer_count++] = layer;
    layer_mark_dirty(layer);

    return layer;
}

void destroy_rain_forecast_layer(Layer *layer) {
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

