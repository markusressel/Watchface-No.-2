#pragma once

#include <stdbool.h>

typedef struct DeveloperOptions {
    bool ShowLayerBounds;
    bool IsEmulator;
} DeveloperOptions;

#ifdef WF_EMULATOR
#define IS_EMULATOR_BUILD true
#else
#define IS_EMULATOR_BUILD false
#endif

#if defined(WF_RELEASE)
// FIXED definition for RELEASE builds to prevent accidentally enabling developer options in a release build
static const DeveloperOptions DEV_OPTIONS __attribute__((unused)) = {
    .ShowLayerBounds = false,
    .IsEmulator = IS_EMULATOR_BUILD,
};
#else

// DEBUG-build-only developer options
static const DeveloperOptions DEV_OPTIONS __attribute__((unused)) = {
    .ShowLayerBounds = false,
    .IsEmulator = IS_EMULATOR_BUILD,
};

#endif
