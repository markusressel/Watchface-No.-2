#pragma once

#include <pebble.h>

#ifdef WF_RELEASE
#undef APP_LOG
#define APP_LOG(level, fmt, args...)
#endif
