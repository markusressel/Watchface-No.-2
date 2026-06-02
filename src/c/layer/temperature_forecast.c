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
#include "../theme.h"
#include "weather.h"

#define MAX_TEMPERATURE_FORECAST_LAYERS 7
#define MAX_FORECAST_POINTS 24

static Layer *s_layers[MAX_TEMPERATURE_FORECAST_LAYERS];
static int s_layer_count = 0;

static size_t bounded_cstring_length(const char *value, const size_t capacity) {
    if (!value || capacity == 0) {
        return 0;
    }

    size_t length = 0;
    while (length < capacity && value[length] != '\0') {
        length++;
    }

    return length;
}

static int parse_int_series(
    const char *encoded,
    const size_t encoded_capacity,
    int *out_values,
    const int max_values
) {
    if (!encoded || encoded_capacity == 0 || !out_values || max_values <= 0 || encoded[0] == '\0') {
        return 0;
    }

    // Never scan beyond the fixed WeatherData buffer even if persisted data is corrupt.
    const size_t encoded_length = bounded_cstring_length(encoded, encoded_capacity);
    if (encoded_length == 0 || encoded_length >= encoded_capacity) {
        return 0;
    }

    int count = 0;
    size_t index = 0;

    while (index < encoded_length && count < max_values) {
        bool is_negative = false;
        if (encoded[index] == '-') {
            is_negative = true;
            index++;
        }

        bool has_digit = false;
        int value = 0;
        while (index < encoded_length && encoded[index] >= '0' && encoded[index] <= '9') {
            has_digit = true;
            value = (value * 10) + (encoded[index] - '0');
            index++;
        }

        if (!has_digit) {
            break;
        }

        out_values[count++] = is_negative ? -value : value;

        if (index >= encoded_length) {
            break;
        }

        if (encoded[index] == ',') {
            index++;
            continue;
        }

        // Unexpected separator, stop parsing remaining values.
        break;
    }

    return count;
}

static void draw_dot(GContext *ctx, const GRect bounds, const int x, const int y, const int dot_size) {
    if (x < 0 || y < 0 || x + dot_size > bounds.size.w || y + dot_size > bounds.size.h) {
        return;
    }

    graphics_fill_rect(ctx, GRect(x, y, dot_size, dot_size), 0, GCornerNone);
}

static void update_proc(Layer *layer, GContext *ctx) {
    const GRect bounds = layer_get_bounds(layer);
    WeatherData *weather_data = weather_get_data();
    ClaySettings *settings = clay_get_settings();

    const int dot_size = settings->DotHeight > 1 ? settings->DotHeight : 1;
    const int point_gap = settings->DotHorizontalGap > 0 ? settings->DotHorizontalGap : 1;

    int values[MAX_FORECAST_POINTS] = {0};
    int value_count = parse_int_series(
        weather_data->TemperatureForecastEncoded,
        sizeof(weather_data->TemperatureForecastEncoded),
        values,
        MAX_FORECAST_POINTS
    );
    if (value_count <= 0) {
        values[0] = weather_data->CurrentTemperature;
        value_count = 1;
    }

    int min_value = values[0];
    int max_value = values[0];
    for (int i = 1; i < value_count; i++) {
        if (values[i] < min_value) {
            min_value = values[i];
        }
        if (values[i] > max_value) {
            max_value = values[i];
        }
    }

    int drawable_height = bounds.size.h - dot_size;
    if (drawable_height < 1) {
        drawable_height = 1;
    }

    graphics_context_set_fill_color(ctx, theme_get_theme()->WeatherTextColor);

    for (int i = 0; i < value_count; i++) {
        int x;
        if (value_count == 1) {
            x = 0;
        } else {
            x = (i * (bounds.size.w - dot_size)) / (value_count - 1);
        }

        int y;
        if (max_value == min_value) {
            y = drawable_height / 2;
        } else {
            const int normalized = (values[i] - min_value) * drawable_height / (max_value - min_value);
            y = drawable_height - normalized;
        }

        draw_dot(ctx, bounds, x, y, dot_size);

        // Add a small horizontal tail to improve readability on sparse point sets.
        if (point_gap > 1 && i < value_count - 1) {
            int tail_x = x + dot_size;
            if (tail_x + dot_size <= bounds.size.w) {
                draw_dot(ctx, bounds, tail_x, y, 1);
            }
        }
    }
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
