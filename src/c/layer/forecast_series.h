#pragma once

#include <stddef.h>

size_t forecast_bounded_cstring_length(const char *value, size_t capacity);

int forecast_parse_int_series(
    const char *encoded,
    size_t encoded_capacity,
    int *out_values,
    int max_values
);

