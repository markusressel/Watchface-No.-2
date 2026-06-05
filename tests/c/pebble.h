#pragma once

#include <stdio.h> // For printf
#include <stdint.h> // For uint32_t (if needed by other Pebble types)

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

// Define common Pebble types if they are used in production code
// that includes pebble.h and is being tested.
typedef uint32_t ResHandle;
// Add other common types like GRect, GPoint, GSize, GColor, etc., as needed
// by the production code being tested.
