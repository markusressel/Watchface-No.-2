#include <pebble.h>
#include "tick_listener.h"
#include "time.h"
//#include "date.h"
#include "clay_settings.h"

static bool registered = false;

// Method to react to tickHandler events (time changes)
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "updating time & date");
  update_date();
  update_time();
}

void register_tick_listener() {
  if (registered) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ignoring register tick listener");
    return;
  } 
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "registering tick listener");
  // Register with TickTimerService
  
  if (clay_get_settings()->ShowSeconds) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }
  
  registered = true;
}

void unregister_tick_listener() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "unregistering tick listener");
  
  // Unregister from TickTimerService
  tick_timer_service_unsubscribe();
  registered = false;
}