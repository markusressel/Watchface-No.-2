#pragma once

typedef struct DeveloperOptions {
    bool ShowLayerBounds;
    bool UseMockWeatherData;
} DeveloperOptions;

// Compile-time developer toggles.
// Set values here manually when debugging visuals.
static const DeveloperOptions DEV_OPTIONS = {
    .ShowLayerBounds = false,
#ifdef PBL_EMULATOR
    .UseMockWeatherData = true,
#else
    .UseMockWeatherData = false,
#endif
};
