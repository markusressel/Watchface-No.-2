#pragma once

// Include the SDK header first, then apply project-wide overrides.
#include_next <pebble.h>

#ifdef RELEASE
#ifdef APP_LOG
#undef APP_LOG
#endif
// Compile out logging calls in release builds, including format strings.
#define APP_LOG(...) ((void)0)
#endif

