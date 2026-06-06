#pragma once

#include <pebble.h>

#ifdef RELEASE
#undef APP_LOG
#define APP_LOG(level, fmt, args...)
#endif
