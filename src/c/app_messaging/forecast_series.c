#include "forecast_series.h"
#include "../ui/graphics/graph_utils.h"
#include "../ui/layer/weather.h"

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
