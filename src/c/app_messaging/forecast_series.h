#pragma once

#include <stddef.h>

int forecast_parse_int_series(
    const char *encoded,
    size_t encoded_capacity,
    int *out_values,
    int max_values
);
