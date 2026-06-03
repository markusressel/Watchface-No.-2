#include "forecast_series.h"
#include "graph_utils.h"
#include "weather.h"
#include "../util.h"

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
        // Skip whitespace or leading commas if any
        while (index < encoded_length && (encoded[index] == ' ' || encoded[index] == ',')) {
            index++;
        }

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
    const GColor defaultColor) {
    // ClaySettings *settings = clay_get_settings();

    // const int dot_size = settings->DotHeight > 1 ? settings->DotHeight : 1;
    const int dot_size = 1;
    // const int min_interpolated_dot_distance = settings->DotHorizontalGap > 0 ? settings->DotHorizontalGap : 1;
    const int min_interpolated_dot_distance = 0;

    int values[maxPoints];
    memset(values, 0, sizeof(values));
    int value_count = weather_data->TemperatureForecastCount;
    if (value_count > maxPoints) {
        value_count = maxPoints;
    }
    if (value_count > 0) {
        memcpy(values, weather_data->TemperatureForecast, sizeof(int) * value_count);
    }
    if (value_count <= 0) {
        values[0] = weather_data->CurrentTemperature;
        value_count = 1;
    }

    char log_prefix[48];
    snprintf(log_prefix, sizeof(log_prefix), "Parsed %d temp values", value_count);
    log_int_array_chunked(log_prefix, values, value_count);

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
    };

    graph_draw_series(ctx, bounds, values, value_count, &graph_config);
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
    // ClaySettings *settings = clay_get_settings();

    // const int dot_size = settings->DotHeight > 1 ? settings->DotHeight : 1;
    const int dot_size = 1;
    // const int min_interpolated_dot_distance = settings->DotHorizontalGap > 0 ? settings->DotHorizontalGap : 1;
    const int min_interpolated_dot_distance = 0;

    int values[maxPoints];
    memset(values, 0, sizeof(values));
    int value_count = weather_data->RainForecastMmX10Count;
    if (value_count > maxPoints) {
        value_count = maxPoints;
    }
    if (value_count > 0) {
        memcpy(values, weather_data->RainForecastMmX10, sizeof(int) * value_count);
    }
    if (value_count <= 0) {
        values[0] = weather_data->RainNextHourMmX10;
        value_count = 1;
    }

    char log_prefix[48];
    snprintf(log_prefix, sizeof(log_prefix), "Parsed %d rain values", value_count);
    log_int_array_chunked(log_prefix, values, value_count);

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
    };

    graph_draw_series(ctx, bounds, values, value_count, &graph_config);
}
