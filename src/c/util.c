#ifndef HOST_TEST_SHIM_BUILD
#include <pebble.h>
#endif
#include <stdio.h>
#include <string.h>

#include "util.h"

void format_int_array(char *buf, size_t buf_len, const int *arr, const int count) {
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

void log_int_array_chunked(const char *prefix, const int *arr, const int count) {
    const int batch_size = 8; // 8 items with commas safely stays way under the 100-char limit

    for (int i = 0; i < count; i += batch_size) {
        char chunk_buf[64];
        size_t written = 0;

        int end = (i + batch_size > count) ? count : i + batch_size;
        for (int j = i; j < end; j++) {
            written += snprintf(chunk_buf + written, sizeof(chunk_buf) - written,
                                "%d%s", arr[j], (j < end - 1) ? ", " : "");
        }

        // Log this batch contextually
#ifndef HOST_TEST_SHIM_BUILD
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s [%d-%d]: [%s]", prefix, i, end - 1, chunk_buf);
#else
        printf("[MOCK APP_LOG] %s [%d-%d]: [%s]\n", prefix, i, end - 1, chunk_buf);
#endif
    }
}
