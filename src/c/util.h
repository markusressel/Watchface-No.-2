#pragma once

#include <stddef.h> // For size_t

void format_int_array(char *buf, size_t buf_len, const int *arr, int count);

void log_int_array_chunked(const char *prefix, const int *arr, int count);
