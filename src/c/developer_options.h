#pragma once

typedef struct DeveloperOptions {
    bool ShowLayerBounds;
} DeveloperOptions;

// Compile-time developer toggles.
// Set values here manually when debugging visuals.
static const DeveloperOptions DEV_OPTIONS = {
    .ShowLayerBounds = true,
};
