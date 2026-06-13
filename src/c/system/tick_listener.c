#include <pebble.h>
#include "tick_listener.h"
#include "../ui/layer/time.h"
#include "../ui/layer/date.h"
#include "../ui/layer/weather_forecast.h"
#include "../settings/clay_settings.h"

static bool registered = false;

// Method to react to tickHandler events (time changes)
static void tick_handler(tm *tick_time, TimeUnits units_changed) {
    (void) tick_time;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "updating time & date");

    if (units_changed & DAY_UNIT) {
        update_date();
    }

    update_time_layer();

    weather_forecast_tick_update();
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