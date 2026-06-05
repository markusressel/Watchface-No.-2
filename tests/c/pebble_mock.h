#pragma once

#include <stdio.h> // For printf
#include <string.h> // For size_t (if not already included by stdio.h)

// Define this to activate conditional compilation in production code
#define HOST_TEST_SHIM_BUILD

// Mock Pebble APP_LOG for host testing
#define APP_LOG_LEVEL_DEBUG   1
#define APP_LOG_LEVEL_INFO    2
#define APP_LOG_LEVEL_WARNING 3
#define APP_LOG_LEVEL_ERROR   4

#define APP_LOG(level, fmt, ...) \
    do { \
        if (level >= APP_LOG_LEVEL_DEBUG) { \
            printf("[MOCK APP_LOG] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while (0)
