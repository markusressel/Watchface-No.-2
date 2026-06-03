#pragma once

#include <stddef.h>
#include "graph_utils.h"
#include "weather.h"

size_t forecast_bounded_cstring_length(const char *value, size_t capacity);

int forecast_parse_int_series(
    const char *encoded,
    size_t encoded_capacity,
    int *out_values,
    int max_values
);

void draw_temperature_forecast_graph(
    GContext *ctx,
    GRect bounds,
    const WeatherData *weather_data,
    int maxPoints,
    GColor defaultColor
);

void draw_rain_forecast_graph(
    GContext *ctx,
    GRect bounds,
    const WeatherData *weather_data,
    int maxPoints,
    GColor defaultColor
);
