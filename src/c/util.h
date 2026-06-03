#pragma once
#include <stdio.h>

inline void format_int_array(char *buf, size_t buf_len, const int *arr, int count) {
    if (buf_len == 0) return;

    size_t written = snprintf(buf, buf_len, "[");
    for (int i = 0; i < count; i++) {
        if (written >= buf_len - 1) break; // Leave room for closing bracket and null terminator

        written += snprintf(buf + written, buf_len - written, "%d%s",
                            arr[i],
                            (i < count - 1) ? ", " : "");
    }

    if (written < buf_len - 1) {
        snprintf(buf + written, buf_len - written, "]");
    } else {
        // Force a closing bracket at the very end if we ran out of space
        buf[buf_len - 2] = ']';
        buf[buf_len - 1] = '\0';
    }
}