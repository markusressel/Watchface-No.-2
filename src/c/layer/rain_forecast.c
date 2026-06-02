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
#include "../theme.h"
#include "weather.h"

#define MAX_RAIN_FORECAST_LAYERS 7
#define MAX_FORECAST_POINTS 24

static Layer *s_layers[MAX_RAIN_FORECAST_LAYERS];
static int s_layer_count = 0;

static int parse_int_series(const char *encoded, int *out_values, const int max_values) {
    if (!encoded || !out_values || max_values <= 0 || encoded[0] == '\0') {
        return 0;
    }

    const char *cursor = encoded;
    int count = 0;

    while (*cursor != '\0' && count < max_values) {
        char *end = NULL;
        long value = strtol(cursor, &end, 10);
        if (end == cursor) {
            break;
        }

        out_values[count++] = (int) value;

        if (*end == ',') {
            cursor = end + 1;
        } else {
            cursor = end;
            if (*cursor != '\0') {
                break;
            }
        }
    }

    return count;
}

static void update_proc(Layer *layer, GContext *ctx) {
    const GRect bounds = layer_get_bounds(layer);
    WeatherData *weather_data = weather_get_data();
    ClaySettings *settings = clay_get_settings();

    const int dot_size = settings->DotHeight > 1 ? settings->DotHeight : 1;

    int values[MAX_FORECAST_POINTS] = {0};
    int value_count = parse_int_series(weather_data->RainForecastMmX10Encoded, values, MAX_FORECAST_POINTS);
    if (value_count <= 0) {
        values[0] = weather_data->RainNextHourMmX10;
        value_count = 1;
    }

    int max_value = 1;
    for (int i = 0; i < value_count; i++) {
        if (values[i] > max_value) {
            max_value = values[i];
        }
    }

    int rows = bounds.size.h / (dot_size + 1);
    if (rows < 1) {
        rows = 1;
    }

    graphics_context_set_fill_color(ctx, theme_get_theme()->WeatherTextColor);

    for (int i = 0; i < value_count; i++) {
        int x;
        if (value_count == 1) {
            x = 0;
        } else {
            x = (i * (bounds.size.w - dot_size)) / (value_count - 1);
        }

        int bar_rows = (values[i] * rows) / max_value;
        if (values[i] > 0 && bar_rows == 0) {
            bar_rows = 1;
        }

        for (int r = 0; r < bar_rows; r++) {
            int y = bounds.size.h - dot_size - (r * (dot_size + 1));
            if (y < 0) {
                break;
            }
            graphics_fill_rect(ctx, GRect(x, y, dot_size, dot_size), 0, GCornerNone);
        }
    }
}

void update_rain_forecast() {
    for (int i = 0; i < s_layer_count; i++) {
        layer_mark_dirty(s_layers[i]);
    }
}

Layer *create_rain_forecast_layer(LayerBuilder builder) {
    if (s_layer_count >= MAX_RAIN_FORECAST_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max rain forecast layers exceeded!");
        return NULL;
    }

    Layer *layer = layer_factory_create_custom_layer(builder, update_proc);
    s_layers[s_layer_count++] = layer;
    layer_mark_dirty(layer);

    return layer;
}

void destroy_rain_forecast_layer(Layer *layer) {
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

