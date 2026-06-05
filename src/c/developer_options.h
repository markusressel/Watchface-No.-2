#pragma once

typedef struct DeveloperOptions {
    bool ShowLayerBounds;
    bool IsEmulator;
} DeveloperOptions;

// Compile-time developer toggles.
// Set values here manually when debugging visuals.
static DeveloperOptions DEV_OPTIONS = {
    .ShowLayerBounds = false,
#ifdef PBL_EMULATOR
    .IsEmulator = true,
#else
    .IsEmulator = false,
#endif
};

static void set_show_layer_bounds(const bool showLayerBounds) {
    DEV_OPTIONS.ShowLayerBounds = showLayerBounds;
}
