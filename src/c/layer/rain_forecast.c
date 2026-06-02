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

#include "../clay_settings.h"
#include "forecast_series.h"
#include "graph_utils.h"
#include "../theme.h"
#include "weather.h"

#define MAX_RAIN_FORECAST_LAYERS 7
#define MAX_FORECAST_POINTS 24

static Layer *s_layers[MAX_RAIN_FORECAST_LAYERS];
static int s_layer_count = 0;

#if defined(PBL_COLOR)
static const GraphColorStop s_rain_color_stops[] = {
    {.value = -50, .color = GColorVividCerulean},
    {.value = -10, .color = GColorPictonBlue},
    {.value = 0, .color = GColorLightGray},
    {.value = 15, .color = GColorPictonBlue},
    {.value = 40, .color = GColorBlueMoon},
    {.value = 80, .color = GColorBlueMoon},
};
#endif

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
        .min_interpolated_dot_distance_px = settings->DotHorizontalGap > 0 ? settings->DotHorizontalGap : 1,
        .fill_area_under_line = false,
        .interpolate_color_stops = true,
        .default_color = theme_get_theme()->WeatherTextColor,
        .color_stops =
#if defined(PBL_COLOR)
            s_rain_color_stops,
#else
            NULL,
#endif
        .color_stop_count =
#if defined(PBL_COLOR)
            (int) (sizeof(s_rain_color_stops) / sizeof(s_rain_color_stops[0])),
#else
            0,
#endif
        .color_for_value = NULL,
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

