#pragma once

#include "settings/clay_settings.h"

// Rebuild the layer layout from the current settings and reload all layers.
// Call this when row layout settings change at runtime.
void main_reload_layout(ClaySettings * settings);