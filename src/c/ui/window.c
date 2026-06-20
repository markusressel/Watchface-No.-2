#include "window.h"

#include <pebble.h>
#include "watch_layout.h"
#include "ui_state.h"
#include "../system/tick_listener.h"
#include "../system/battery_listener.h"
#include "../system/health_listener.h"
#include "../system/phone_connection.h"
#include "layer/status.h"
#include "layer/weather.h"

// loads components into the main window
void window_load(Window *window) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_load");

    ui_state_create_layers(watch_layout_get_layout());
    status_layer_create(window_get_root_layer(window));

    // Register for tick events (time)
    register_tick_listener();
    // Register for battery level updates
    register_battery_listener();
    // Ensure battery level is displayed from the start
    force_battery_update();

    // Register health event listener
    register_health_event_listener();

    // Register for phone connection updates
    register_phone_connection_listener();
    force_phone_connection_update();

    // Check if weather update is needed on launch
    weather_check_and_request_update();
}

// destroys all components of the main window
void window_unload(Window *window) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_unload");
    // unregister listeners
    unregister_tick_listener();
    unregister_battery_listener();
    unregister_health_event_listener();
    unregister_phone_connection_listener();

    status_layer_destroy();
    ui_state_destroy_layers();
}
