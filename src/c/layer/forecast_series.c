#include "forecast_series.h"
#include "graph_utils.h"
#include "weather.h"

static size_t forecast_bounded_cstring_length(const char *value, const size_t capacity) {
    if (!value || capacity == 0) {
        return 0;
    }

    size_t length = 0;
    while (length < capacity && value[length] != '\0') {
        length++;
    }

    return length;
}

int forecast_parse_int_series(
    const char *encoded,
    const size_t encoded_capacity,
    int *out_values,
    const int max_values
) {
    if (!encoded || encoded_capacity == 0 || !out_values || max_values <= 0 || encoded[0] == '\0') {
        return 0;
    }

    // Never scan beyond the fixed WeatherData buffer even if persisted data is corrupt.
    const size_t encoded_length = forecast_bounded_cstring_length(encoded, encoded_capacity);
    if (encoded_length == 0 || encoded_length >= encoded_capacity) {
        return 0;
    }

    int count = 0;
    size_t index = 0;

    while (index < encoded_length && count < max_values) {
        // Skip whitespace and delimiters.
        while (index < encoded_length && (encoded[index] == ' ' || encoded[index] == ',')) {
            index++;
        }
        if (index >= encoded_length) {
            break;
        }

        bool is_negative = false;
        if (encoded[index] == '-') {
            is_negative = true;
            index++;
        }
        if (index >= encoded_length) {
            break;
        }

        bool has_digit = false;
        int value = 0;
        while (index < encoded_length && encoded[index] >= '0' && encoded[index] <= '9') {
            has_digit = true;
            value = (value * 10) + (encoded[index] - '0');
            index++;
        }

        if (!has_digit) {
            // Ignore malformed token content and continue with next comma-separated value.
            while (index < encoded_length && encoded[index] != ',') {
                index++;
            }
            continue;
        }

        out_values[count++] = is_negative ? -value : value;

        // If a token has trailing junk (e.g. "12abc"), skip to next separator.
        while (index < encoded_length && encoded[index] != ',' && encoded[index] != ' ') {
            index++;
        }
    }

    return count;
}


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

void draw_temperature_forecast_graph(
    GContext *ctx,
    const GRect bounds,
    const WeatherData *weather_data,
    const int maxPoints,
    const GColor defaultColor
) {
    const int dot_size = 1;
    const int min_interpolated_dot_distance = 0;

    if (!weather_data) {
        return;
    }

    int value_count = weather_data->TemperatureForecastCount;
    const int *render_values = weather_data->TemperatureForecast;
    int fallback_value[1]; // Ultra-lightweight fallback array

    // Defensive check: if count is zero OR the pointer is missing, drop to fallback
    if (value_count <= 0 || !render_values) {
        fallback_value[0] = weather_data->CurrentTemperature;
        render_values = fallback_value;
        value_count = 1;
    } else if (value_count > maxPoints) {
        value_count = maxPoints;
    }

    GraphDrawConfig graph_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = dot_size,
        .min_interpolated_dot_distance_px = min_interpolated_dot_distance,
        .fill_area_under_line = false,
        .suppress_exact_zero_value = false,
        .interpolate_color_stops = true,
        .default_color = defaultColor,
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
        .has_y_axis_range = false,
        .y_min = 0,
        .y_max = 0,
    };

    GraphInstance graph_instance;
    graph_instance_init(&graph_instance, render_values, value_count);
    graph_instance_set_config(&graph_instance, &graph_config);
    graph_instance_draw(&graph_instance, ctx, bounds);
}


#if defined(PBL_COLOR)
static const GraphColorStop s_rain_color_stops[] = {
    {.value = -50, .color = GColorVividCerulean},
    {.value = -10, .color = GColorPictonBlue},
    {.value = 0, .color = GColorLightGray},
    {.value = 3, .color = GColorPictonBlue},
    {.value = 10, .color = GColorBlueMoon},
    {.value = 50, .color = GColorBlue},
    {.value = 100, .color = GColorDukeBlue},
};
#endif

void draw_rain_forecast_graph(
    GContext *ctx,
    const GRect bounds,
    const WeatherData *weather_data,
    const int maxPoints,
    const GColor defaultColor
) {
    const int dot_size = 1;
    const int min_interpolated_dot_distance = 0;

    if (!weather_data) {
        return;
    }

    int value_count = weather_data->RainForecastMmX10Count;
    const int *render_values = weather_data->RainForecastMmX10;
    int fallback_value[1]; // Ultra-lightweight fallback array

    // Defensive check: if count is zero OR the pointer is missing, drop to fallback
    if (value_count <= 0 || !render_values) {
        fallback_value[0] = weather_data->RainNextHourMmX10;
        render_values = fallback_value;
        value_count = 1;
    } else if (value_count > maxPoints) {
        value_count = maxPoints;
    }

    GraphDrawConfig graph_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = dot_size,
        .min_interpolated_dot_distance_px = min_interpolated_dot_distance,
        .fill_area_under_line = true,
        .suppress_exact_zero_value = true,
        .interpolate_color_stops = false,
        .default_color = defaultColor,
#if defined(PBL_COLOR)
        .color_stops = s_rain_color_stops,
#else
        .color_stops = NULL,
#endif
        .color_stop_count =
#if defined(PBL_COLOR)
        (int) (sizeof(s_rain_color_stops) / sizeof(s_rain_color_stops[0])),
#else
        0,
#endif
        .color_for_value = NULL,
        .color_context = NULL,
        .has_y_axis_range = false,
        .y_min = 0,
        .y_max = 0,
    };

    GraphInstance graph_instance;
    graph_instance_init(&graph_instance, render_values, value_count);
    graph_instance_set_config(&graph_instance, &graph_config);
    graph_instance_set_y_axis_range(&graph_instance, 0, 50); // 5mm
    graph_instance_draw(&graph_instance, ctx, bounds);
}
