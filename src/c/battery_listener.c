#include <pebble.h>
#define EXTERN
#include "battery.h"
#include "battery_listener.h"
#include "battery_bar.h"

static bool registered = false;

static int s_current_battery_level = -1;

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  s_battery_charging = state.is_charging;
  s_battery_cable_connected = state.is_plugged;
  
  if (s_current_battery_level == s_battery_level) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Ignoring already set battery value: %d", (int) s_battery_level);
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery changed: old: %d, new: %d", s_current_battery_level, (int) s_battery_level);
    s_current_battery_level = s_battery_level;
    
    // informing listeners about change
    update_battery_bar();
  }
}

void register_battery_listener() {
  if (registered) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ignoring register battery listener");
    return;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "registering battery listener");
  
  battery_state_service_subscribe(battery_callback);
  registered = true;
}

void unregister_battery_listener() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "unregistering battery listener");
  
  battery_state_service_unsubscribe();
  registered = false;
}

void force_battery_update() {
  battery_callback(battery_state_service_peek());
}