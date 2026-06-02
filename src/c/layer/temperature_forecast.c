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

#include "../clay_settings.h"
#include "forecast_series.h"
#include "graph_utils.h"
#include "../theme.h"
#include "weather.h"

#define MAX_TEMPERATURE_FORECAST_LAYERS 7
#define MAX_FORECAST_POINTS 24

static Layer *s_layers[MAX_TEMPERATURE_FORECAST_LAYERS];
static int s_layer_count = 0;

#if defined(PBL_COLOR)
static const GraphColorStop s_temperature_color_stops[] = {
    {.value = -25, .color = GColorMagenta},
    {.value = -12, .color = GColorBlueMoon},
    {.value = -1, .color = GColorPictonBlue},
    {.value = 0, .color = GColorGreen},
    {.value = 15, .color = GColorChromeYellow},
    {.value = 30, .color = GColorRed},
};
#endif

static void update_proc(Layer *layer, GContext *ctx) {
    const GRect bounds = layer_get_bounds(layer);
    WeatherData *weather_data = weather_get_data();
    ClaySettings *settings = clay_get_settings();

    // const int dot_size = settings->DotHeight > 1 ? settings->DotHeight : 1;
    const int dot_size = 1;
    // const int min_interpolated_dot_distance = settings->DotHorizontalGap > 0 ? settings->DotHorizontalGap : 1;
    const int min_interpolated_dot_distance = 0;

    int values[MAX_FORECAST_POINTS] = {0};
    int value_count = forecast_parse_int_series(
        weather_data->TemperatureForecastEncoded,
        sizeof(weather_data->TemperatureForecastEncoded),
        values,
        MAX_FORECAST_POINTS
    );
    if (value_count <= 0) {
        values[0] = weather_data->CurrentTemperature;
        value_count = 1;
    }

    GraphDrawConfig graph_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = dot_size,
        .min_interpolated_dot_distance_px = min_interpolated_dot_distance,
        .fill_area_under_line = true,
        .interpolate_color_stops = true,
        .default_color = theme_get_theme()->WeatherTextColor,
        .color_stops =
#if defined(PBL_COLOR)
            s_temperature_color_stops,
#else
            NULL,
#endif
        .color_stop_count =
#if defined(PBL_COLOR)
            (int) (sizeof(s_temperature_color_stops) / sizeof(s_temperature_color_stops[0])),
#else
            0,
#endif
        .color_for_value = NULL,
        .color_context = NULL,
    };

    graph_draw_series(ctx, bounds, values, value_count, &graph_config);
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
