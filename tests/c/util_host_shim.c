#define HOST_TEST_SHIM_BUILD

#include <stdio.h> // For printf
#include <string.h> // For size_t (if not already included by stdio.h)

// Mock Pebble APP_LOG for host testing
// This mock is primarily for other files that might include this shim
// and use APP_LOG. For util.c itself, the conditional compilation
// within util.c now handles the APP_LOG replacement.
#define APP_LOG_LEVEL_DEBUG   1
#define APP_LOG_LEVEL_INFO    2
#define APP_LOG_LEVEL_WARNING 3
#define APP_LOG_LEVEL_ERROR   4

#define APP_LOG(level, fmt, ...) \
    do { \
        if (level >= APP_LOG_LEVEL_DEBUG) { \
            printf("[MOCK APP_LOG from shim] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while (0)

// Include the original util.c directly to compile its functions
// in the context of this host test shim.
#include "../../src/c/util.c"
